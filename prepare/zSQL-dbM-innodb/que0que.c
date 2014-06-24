/*****************************************************************************

Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/**************************************************//**
@file que/que0que.c
Query graph

Created 5/27/1996 Heikki Tuuri
*******************************************************/

#include "que0que.h"

#ifdef UNIV_NONINL
#include "que0que.ic"
#endif

#include "usr0sess.h"
#include "trx0trx.h"
#include "trx0roll.h"
#include "row0undo.h"
#include "row0ins.h"
#include "row0upd.h"
#include "row0sel.h"
#include "row0purge.h"
#include "dict0crea.h"
#include "log0log.h"
#include "eval0proc.h"
#include "eval0eval.h"
#include "pars0types.h"

#define QUE_PARALLELIZE_LIMIT	(64 * 256 * 256 * 256)
#define QUE_ROUND_ROBIN_LIMIT	(64 * 256 * 256 * 256)
#define QUE_MAX_LOOPS_WITHOUT_CHECK	16

#ifdef UNIV_DEBUG
/* If the following flag is set TRUE, the module will print trace info
of SQL execution in the UNIV_SQL_DEBUG version */
UNIV_INTERN ibool	que_trace_on		= FALSE;
#endif /* UNIV_DEBUG */

/* Short introduction to query graphs
   ==================================

A query graph consists of nodes linked to each other in various ways. The
execution starts at que_run_threads() which takes a que_thr_t parameter.
que_thr_t contains two fields that control query graph execution: run_node
and prev_node. run_node is the next node to execute and prev_node is the
last node executed.

Each node has a pointer to a 'next' statement, i.e., its brother, and a
pointer to its parent node. The next pointer is NULL in the last statement
of a block.

Loop nodes contain a link to the first statement of the enclosed statement
list. While the loop runs, que_thr_step() checks if execution to the loop
node came from its parent or from one of the statement nodes in the loop. If
it came from the parent of the loop node it starts executing the first
statement node in the loop. If it came from one of the statement nodes in
the loop, then it checks if the statement node has another statement node
following it, and runs it if so.

To signify loop ending, the loop statements (see e.g. while_step()) set
que_thr_t->run_node to the loop node's parent node. This is noticed on the
next call of que_thr_step() and execution proceeds to the node pointed to by
the loop node's 'next' pointer.

For example, the code:

X := 1;
WHILE X < 5 LOOP
 X := X + 1;
 X := X + 1;
X := 5

will result in the following node hierarchy, with the X-axis indicating
'next' links and the Y-axis indicating parent/child links:

A - W - A
    |
    |
    A - A

A = assign_node_t, W = while_node_t. */

/* How a stored procedure containing COMMIT or ROLLBACK commands
is executed?

The commit or rollback can be seen as a subprocedure call.
The problem is that if there are several query threads
currently running within the transaction, their action could
mess the commit or rollback operation. Or, at the least, the
operation would be difficult to visualize and keep in control.

Therefore the query thread requesting a commit or a rollback
sends to the transaction a signal, which moves the transaction
to TRX_QUE_SIGNALED state. All running query threads of the
transaction will eventually notice that the transaction is now in
this state and voluntarily suspend themselves. Only the last
query thread which suspends itself will trigger handling of
the signal.

When the transaction starts to handle a rollback or commit
signal, it builds a query graph which, when executed, will
roll back or commit the incomplete transaction. The transaction
is moved to the TRX_QUE_ROLLING_BACK or TRX_QUE_COMMITTING state.
If specified, the SQL cursors opened by the transaction are closed.
When the execution of the graph completes, it is like returning
from a subprocedure: the query thread which requested the operation
starts running again. */

/**********************************************************************//**
Moves a thread from another state to the QUE_THR_RUNNING state. Increments
the n_active_thrs counters of the query graph and transaction.
***NOTE***: This is the only function in which such a transition is allowed
to happen! */
static
void
que_thr_move_to_run_state(
/*======================*/
	que_thr_t*	thr);	/*!< in: an query thread */

/***********************************************************************//**
Adds a query graph to the session's list of graphs. */
UNIV_INTERN
void
que_graph_publish(
/*==============*/
	que_t*	graph,	/*!< in: graph */
	sess_t*	sess)	/*!< in: session */
{
	ut_ad(mutex_own(&kernel_mutex));

	UT_LIST_ADD_LAST(graphs, sess->graphs, graph);
}

/***********************************************************************//**
Creates a query graph fork node.
@return	own: fork node */
UNIV_INTERN
que_fork_t*
que_fork_create(
/*============*/
	que_t*		graph,		/*!< in: graph, if NULL then this
					fork node is assumed to be the
					graph root */
	que_node_t*	parent,		/*!< in: parent node */
	ulint		fork_type,	/*!< in: fork type */
	mem_heap_t*	heap)		/*!< in: memory heap where created */
{
	que_fork_t*	fork;

	ut_ad(heap);

	fork = mem_heap_alloc(heap, sizeof(que_fork_t));

	fork->common.type = QUE_NODE_FORK;
	fork->n_active_thrs = 0;

	fork->state = QUE_FORK_COMMAND_WAIT;

	if (graph != NULL) {
		fork->graph = graph;
	} else {
		fork->graph = fork;
	}

	fork->common.parent = parent;
	fork->fork_type = fork_type;

	fork->caller = NULL;

	UT_LIST_INIT(fork->thrs);

	fork->sym_tab = NULL;
	fork->info = NULL;

	fork->heap = heap;

	return(fork);
}

/***********************************************************************//**
Creates a query graph thread node.
@return	own: query thread node */
UNIV_INTERN
que_thr_t*
que_thr_create(
/*===========*/
	que_fork_t*	parent,	/*!< in: parent node, i.e., a fork node */
	mem_heap_t*	heap)	/*!< in: memory heap where created */
{
	que_thr_t*	thr;

	ut_ad(parent && heap);

	thr = mem_heap_alloc(heap, sizeof(que_thr_t));

	thr->common.type = QUE_NODE_THR;
	thr->common.parent = parent;

	thr->magic_n = QUE_THR_MAGIC_N;

	thr->graph = parent->graph;

	thr->state = QUE_THR_COMMAND_WAIT;

	thr->is_active = FALSE;

	thr->run_node = NULL;
	thr->resource = 0;
	thr->lock_state = QUE_THR_LOCK_NOLOCK;

	UT_LIST_ADD_LAST(thrs, parent->thrs, thr);

	return(thr);
}

/**********************************************************************//**
Moves a suspended query thread to the QUE_THR_RUNNING state and may release
a single worker thread to execute it. This function should be used to end
the wait state of a query thread waiting for a lock or a stored procedure
completion. */
UNIV_INTERN
void
que_thr_end_wait(
/*=============*/
	que_thr_t*	thr,		/*!< in: query thread in the
					QUE_THR_LOCK_WAIT,
					or QUE_THR_PROCEDURE_WAIT, or
					QUE_THR_SIG_REPLY_WAIT state */
	que_thr_t**	next_thr)	/*!< in/out: next query thread to run;
					if the value which is passed in is
					a pointer to a NULL pointer, then the
					calling function can start running
					a new query thread; if NULL is passed
					as the parameter, it is ignored */
{
	ibool	was_active;

	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(thr);
	ut_ad((thr->state == QUE_THR_LOCK_WAIT)
	      || (thr->state == QUE_THR_PROCEDURE_WAIT)
	      || (thr->state == QUE_THR_SIG_REPLY_WAIT));
	ut_ad(thr->run_node);

	thr->prev_node = thr->run_node;

	was_active = thr->is_active;

	que_thr_move_to_run_state(thr);

	if (was_active) {

		return;
	}

	if (next_thr && *next_thr == NULL) {
		*next_thr = thr;
	} else {
		ut_a(0);
		srv_que_task_enqueue_low(thr);
	}
}

/**********************************************************************//**
Same as que_thr_end_wait, but no parameter next_thr available. */
UNIV_INTERN
void
que_thr_end_wait_no_next_thr(
/*=========================*/
	que_thr_t*	thr)	/*!< in: query thread in the QUE_THR_LOCK_WAIT,
				or QUE_THR_PROCEDURE_WAIT, or
				QUE_THR_SIG_REPLY_WAIT state */
{
	ibool	was_active;

	ut_a(thr->state == QUE_THR_LOCK_WAIT);	/* In MySQL this is the
						only possible state here */
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(thr);
	ut_ad((thr->state == QUE_THR_LOCK_WAIT)
	      || (thr->state == QUE_THR_PROCEDURE_WAIT)
	      || (thr->state == QUE_THR_SIG_REPLY_WAIT));

	was_active = thr->is_active;

	que_thr_move_to_run_state(thr);

	if (was_active) {

		return;
	}

	/* In MySQL we let the OS thread (not just the query thread) to wait
	for the lock to be released: */

	srv_release_mysql_thread_if_suspended(thr);

	/* srv_que_task_enqueue_low(thr); */
}

/**********************************************************************//**
Inits a query thread for a command. */
UNIV_INLINE
void
que_thr_init_command(
/*=================*/
	que_thr_t*	thr)	/*!< in: query thread */
{
	thr->run_node = thr;
	thr->prev_node = thr->common.parent;

	que_thr_move_to_run_state(thr);
}

/**********************************************************************//**
Starts execution of a command in a query fork. Picks a query thread which
is not in the QUE_THR_RUNNING state and moves it to that state. If none
can be chosen, a situation which may arise in parallelized fetches, NULL
is returned.
@return a query thread of the graph moved to QUE_THR_RUNNING state, or
NULL; the query thread should be executed by que_run_threads by the
caller */
UNIV_INTERN
que_thr_t*
que_fork_start_command(
/*===================*/
	que_fork_t*	fork)	/*!< in: a query fork */
{
	que_thr_t*	thr;
	que_thr_t*	suspended_thr = NULL;
	que_thr_t*	completed_thr = NULL;

	fork->state = QUE_FORK_ACTIVE;

	fork->last_sel_node = NULL;

	suspended_thr = NULL;
	completed_thr = NULL;

	/* Choose the query thread to run: usually there is just one thread,
	but in a parallelized select, which necessarily is non-scrollable,
	there may be several to choose from */

	/* First we try to find a query thread in the QUE_THR_COMMAND_WAIT
	state. Then we try to find a query thread in the QUE_THR_SUSPENDED
	state, finally we try to find a query thread in the QUE_THR_COMPLETED
	state */

	thr = UT_LIST_GET_FIRST(fork->thrs);

	/* We make a single pass over the thr list within which we note which
	threads are ready to run. */
	while (thr) {
		switch (thr->state) {
		case QUE_THR_COMMAND_WAIT:

			/* We have to send the initial message to query thread
			to start it */

			que_thr_init_command(thr); //->“thr->run_node = thr;..thr->prev_node = thr->common.parent; .. que_thr_move_to_run_state(thr);” //----2011-10-12-19-13; //que_thr_move_to_run_state:-->“thr->state = QUE_THR_RUNNING;”//----2011-10-12-19-15;

			return(thr);

		case QUE_THR_SUSPENDED:
			/* In this case the execution of the thread was
			suspended: no initial message is needed because
			execution can continue from where it was left */
			if (!suspended_thr) {
				suspended_thr = thr;
			}

			break;

		case QUE_THR_COMPLETED:
			if (!completed_thr) {
				completed_thr = thr;
			}

			break;

		case QUE_THR_LOCK_WAIT:
			ut_error;

		}

		thr = UT_LIST_GET_NEXT(thrs, thr);
	}

	if (suspended_thr) {

		thr = suspended_thr;
		que_thr_move_to_run_state(thr);

	} else if (completed_thr) {

		thr = completed_thr;
		que_thr_init_command(thr);
	}

	return(thr);
}

/**********************************************************************//**
After signal handling is finished, returns control to a query graph error
handling routine. (Currently, just returns the control to the root of the
graph so that the graph can communicate an error message to the client.) */
UNIV_INTERN
void
que_fork_error_handle(
/*==================*/
	trx_t*	trx __attribute__((unused)),	/*!< in: trx */
	que_t*	fork)	/*!< in: query graph which was run before signal
			handling started, NULL not allowed */
{
	que_thr_t*	thr;

	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(trx->sess->state == SESS_ERROR);
	ut_ad(UT_LIST_GET_LEN(trx->reply_signals) == 0);
	ut_ad(UT_LIST_GET_LEN(trx->wait_thrs) == 0);

	thr = UT_LIST_GET_FIRST(fork->thrs);

	while (thr != NULL) {
		ut_ad(!thr->is_active);
		ut_ad(thr->state != QUE_THR_SIG_REPLY_WAIT);
		ut_ad(thr->state != QUE_THR_LOCK_WAIT);

		thr->run_node = thr;
		thr->prev_node = thr->child;
		thr->state = QUE_THR_COMPLETED;

		thr = UT_LIST_GET_NEXT(thrs, thr);
	}

	thr = UT_LIST_GET_FIRST(fork->thrs);

	que_thr_move_to_run_state(thr);

	ut_a(0);
	srv_que_task_enqueue_low(thr);
}

/****************************************************************//**
Tests if all the query threads in the same fork have a given state.
@return TRUE if all the query threads in the same fork were in the
given state */
UNIV_INLINE
ibool
que_fork_all_thrs_in_state(
/*=======================*/
	que_fork_t*	fork,	/*!< in: query fork */
	ulint		state)	/*!< in: state */
{
	que_thr_t*	thr_node;

	thr_node = UT_LIST_GET_FIRST(fork->thrs);

	while (thr_node != NULL) {
		if (thr_node->state != state) {

			return(FALSE);
		}

		thr_node = UT_LIST_GET_NEXT(thrs, thr_node);
	}

	return(TRUE);
}

/**********************************************************************//**
Calls que_graph_free_recursive for statements in a statement list. */
static
void
que_graph_free_stat_list(
/*=====================*/
	que_node_t*	node)	/*!< in: first query graph node in the list */
{
	while (node) {
		que_graph_free_recursive(node);

		node = que_node_get_next(node);
	}
}

/**********************************************************************//**
Frees a query graph, but not the heap where it was created. Does not free
explicit cursor declarations, they are freed in que_graph_free. */
UNIV_INTERN
void
que_graph_free_recursive(
/*=====================*/
	que_node_t*	node)	/*!< in: query graph node */
{
	que_fork_t*	fork;
	que_thr_t*	thr;
	undo_node_t*	undo;
	sel_node_t*	sel;
	ins_node_t*	ins;
	upd_node_t*	upd;
	tab_node_t*	cre_tab;
	ind_node_t*	cre_ind;
	purge_node_t*	purge;

	if (node == NULL) {

		return;
	}

	switch (que_node_get_type(node)) {

	case QUE_NODE_FORK:
		fork = node;

		thr = UT_LIST_GET_FIRST(fork->thrs);

		while (thr) {
			que_graph_free_recursive(thr);

			thr = UT_LIST_GET_NEXT(thrs, thr);
		}

		break;
	case QUE_NODE_THR:

		thr = node;

		if (thr->magic_n != QUE_THR_MAGIC_N) {
			fprintf(stderr,
				"que_thr struct appears corrupt;"
				" magic n %lu\n",
				(unsigned long) thr->magic_n);
			mem_analyze_corruption(thr);
			ut_error;
		}

		thr->magic_n = QUE_THR_MAGIC_FREED;

		que_graph_free_recursive(thr->child);

		break;
	case QUE_NODE_UNDO:

		undo = node;

		mem_heap_free(undo->heap);

		break;
	case QUE_NODE_SELECT:

		sel = node;

		sel_node_free_private(sel);

		break;
	case QUE_NODE_INSERT:

		ins = node;

		que_graph_free_recursive(ins->select);

		mem_heap_free(ins->entry_sys_heap);

		break;
	case QUE_NODE_PURGE:
		purge = node;

		mem_heap_free(purge->heap);

		break;

	case QUE_NODE_UPDATE:

		upd = node;

		if (upd->in_mysql_interface) {

			btr_pcur_free_for_mysql(upd->pcur);
		}

		que_graph_free_recursive(upd->cascade_node);

		if (upd->cascade_heap) {
			mem_heap_free(upd->cascade_heap);
		}

		que_graph_free_recursive(upd->select);

		mem_heap_free(upd->heap);

		break;
	case QUE_NODE_CREATE_TABLE:
		cre_tab = node;

		que_graph_free_recursive(cre_tab->tab_def);
		que_graph_free_recursive(cre_tab->col_def);
		que_graph_free_recursive(cre_tab->commit_node);

		mem_heap_free(cre_tab->heap);

		break;
	case QUE_NODE_CREATE_INDEX:
		cre_ind = node;

		que_graph_free_recursive(cre_ind->ind_def);
		que_graph_free_recursive(cre_ind->field_def);
		que_graph_free_recursive(cre_ind->commit_node);

		mem_heap_free(cre_ind->heap);

		break;
	case QUE_NODE_PROC:
		que_graph_free_stat_list(((proc_node_t*)node)->stat_list);

		break;
	case QUE_NODE_IF:
		que_graph_free_stat_list(((if_node_t*)node)->stat_list);
		que_graph_free_stat_list(((if_node_t*)node)->else_part);
		que_graph_free_stat_list(((if_node_t*)node)->elsif_list);

		break;
	case QUE_NODE_ELSIF:
		que_graph_free_stat_list(((elsif_node_t*)node)->stat_list);

		break;
	case QUE_NODE_WHILE:
		que_graph_free_stat_list(((while_node_t*)node)->stat_list);

		break;
	case QUE_NODE_FOR:
		que_graph_free_stat_list(((for_node_t*)node)->stat_list);

		break;

	case QUE_NODE_ASSIGNMENT:
	case QUE_NODE_EXIT:
	case QUE_NODE_RETURN:
	case QUE_NODE_COMMIT:
	case QUE_NODE_ROLLBACK:
	case QUE_NODE_LOCK:
	case QUE_NODE_FUNC:
	case QUE_NODE_ORDER:
	case QUE_NODE_ROW_PRINTF:
	case QUE_NODE_OPEN:
	case QUE_NODE_FETCH:
		/* No need to do anything */

		break;
	default:
		fprintf(stderr,
			"que_node struct appears corrupt; type %lu\n",
			(unsigned long) que_node_get_type(node));
		mem_analyze_corruption(node);
		ut_error;
	}
}

/**********************************************************************//**
Frees a query graph. */
UNIV_INTERN
void
que_graph_free(
/*===========*/
	que_t*	graph)	/*!< in: query graph; we assume that the memory
			heap where this graph was created is private
			to this graph: if not, then use
			que_graph_free_recursive and free the heap
			afterwards! */
{
	ut_ad(graph);

	if (graph->sym_tab) {
		/* The following call frees dynamic memory allocated
		for variables etc. during execution. Frees also explicit
		cursor definitions. */

		sym_tab_free_private(graph->sym_tab);
	}

	if (graph->info && graph->info->graph_owns_us) {
		pars_info_free(graph->info);
	}

	que_graph_free_recursive(graph);

	mem_heap_free(graph->heap);
}

/****************************************************************//**
Performs an execution step on a thr node.
@return	query thread to run next, or NULL if none */
static
que_thr_t*
que_thr_node_step(
/*==============*/
	que_thr_t*	thr)	/*!< in: query thread where run_node must
				be the thread node itself */
{
	ut_ad(thr->run_node == thr);

	if (thr->prev_node == thr->common.parent) {
		/* If control to the node came from above, it is just passed
		on */

		thr->run_node = thr->child;

		return(thr);
	}

	mutex_enter(&kernel_mutex);

	if (que_thr_peek_stop(thr)) {

		mutex_exit(&kernel_mutex);

		return(thr);
	}

	/* Thread execution completed */

	thr->state = QUE_THR_COMPLETED;

	mutex_exit(&kernel_mutex);

	return(NULL);
}

/**********************************************************************//**
Moves a thread from another state to the QUE_THR_RUNNING state. Increments
the n_active_thrs counters of the query graph and transaction if thr was
not active.
***NOTE***: This and ..._mysql are  the only functions in which such a
transition is allowed to happen! */
static
void
que_thr_move_to_run_state(
/*======================*/
	que_thr_t*	thr)	/*!< in: an query thread */
{
	trx_t*	trx;

	ut_ad(thr->state != QUE_THR_RUNNING);

	trx = thr_get_trx(thr);

	if (!thr->is_active) {

		(thr->graph)->n_active_thrs++;

		trx->n_active_thrs++;

		thr->is_active = TRUE;

		ut_ad((thr->graph)->n_active_thrs == 1);
		ut_ad(trx->n_active_thrs == 1);
	}

	thr->state = QUE_THR_RUNNING;
}

/**********************************************************************//**
Decrements the query thread reference counts in the query graph and the
transaction. May start signal handling, e.g., a rollback.
*** NOTE ***:
This and que_thr_stop_for_mysql are the only functions where the reference
count can be decremented and this function may only be called from inside
que_run_threads or que_thr_check_if_switch! These restrictions exist to make
the rollback code easier to maintain. */
static
void
que_thr_dec_refer_count(
/*====================*/
	que_thr_t*	thr,		/*!< in: query thread */
	que_thr_t**	next_thr)	/*!< in/out: next query thread to run;
					if the value which is passed in is
					a pointer to a NULL pointer, then the
					calling function can start running
					a new query thread */
{
	que_fork_t*	fork;
	trx_t*		trx;
	ulint		fork_type;
	ibool		stopped;

	fork = thr->common.parent;
	trx = thr_get_trx(thr);

	mutex_enter(&kernel_mutex);

	ut_a(thr->is_active);

	if (thr->state == QUE_THR_RUNNING) {

		stopped = que_thr_stop(thr);

		if (!stopped) {
			/* The reason for the thr suspension or wait was
			already canceled before we came here: continue
			running the thread */

			/* fputs("!!!!!!!! Wait already ended: continue thr\n",
			stderr); */

			if (next_thr && *next_thr == NULL) {
				/* Normally srv_suspend_mysql_thread resets
				the state to DB_SUCCESS before waiting, but
				in this case we have to do it here,
				otherwise nobody does it. */
				trx->error_state = DB_SUCCESS;

				*next_thr = thr;
			} else {
				ut_error;
				srv_que_task_enqueue_low(thr);
			}

			mutex_exit(&kernel_mutex);

			return;
		}
	}

	ut_ad(fork->n_active_thrs == 1);
	ut_ad(trx->n_active_thrs == 1);

	fork->n_active_thrs--;
	trx->n_active_thrs--;

	thr->is_active = FALSE;

	if (trx->n_active_thrs > 0) {

		mutex_exit(&kernel_mutex);

		return;
	}

	fork_type = fork->fork_type;

	/* Check if all query threads in the same fork are completed */

	if (que_fork_all_thrs_in_state(fork, QUE_THR_COMPLETED)) {

		switch (fork_type) {
		case QUE_FORK_ROLLBACK:
			/* This is really the undo graph used in rollback,
			no roll_node in this graph */

			ut_ad(UT_LIST_GET_LEN(trx->signals) > 0);
			ut_ad(trx->handling_signals == TRUE);

			trx_finish_rollback_off_kernel(fork, trx, next_thr);
			break;

		case QUE_FORK_PURGE:
		case QUE_FORK_RECOVERY:
		case QUE_FORK_MYSQL_INTERFACE:

			/* Do nothing */
			break;

		default:
			ut_error;	/*!< not used in MySQL */
		}
	}

	if (UT_LIST_GET_LEN(trx->signals) > 0 && trx->n_active_thrs == 0) {

		/* If the trx is signaled and its query thread count drops to
		zero, then we start processing a signal; from it we may get
		a new query thread to run */

		trx_sig_start_handle(trx, next_thr);
	}

	if (trx->handling_signals && UT_LIST_GET_LEN(trx->signals) == 0) {

		trx_end_signal_handling(trx);
	}

	mutex_exit(&kernel_mutex);
}

/**********************************************************************//**
Stops a query thread if graph or trx is in a state requiring it. The
conditions are tested in the order (1) graph, (2) trx. The kernel mutex has
to be reserved.
@return	TRUE if stopped */
UNIV_INTERN
ibool
que_thr_stop(
/*=========*/
	que_thr_t*	thr)	/*!< in: query thread */
{
	trx_t*	trx;
	que_t*	graph;
	ibool	ret	= TRUE;

	ut_ad(mutex_own(&kernel_mutex));

	graph = thr->graph;
	trx = graph->trx;

	if (graph->state == QUE_FORK_COMMAND_WAIT) {
		thr->state = QUE_THR_SUSPENDED;

	} else if (trx->que_state == TRX_QUE_LOCK_WAIT) {

		UT_LIST_ADD_FIRST(trx_thrs, trx->wait_thrs, thr);
		thr->state = QUE_THR_LOCK_WAIT;

	} else if (trx->error_state != DB_SUCCESS
		   && trx->error_state != DB_LOCK_WAIT) {

		/* Error handling built for the MySQL interface */
		thr->state = QUE_THR_COMPLETED;

	} else if (UT_LIST_GET_LEN(trx->signals) > 0
		   && graph->fork_type != QUE_FORK_ROLLBACK) {

		thr->state = QUE_THR_SUSPENDED;
	} else {
		ut_ad(graph->state == QUE_FORK_ACTIVE);

		ret = FALSE;
	}

	return(ret);
}

/**********************************************************************//**
A patch for MySQL used to 'stop' a dummy query thread used in MySQL. The
query thread is stopped and made inactive, except in the case where
it was put to the lock wait state in lock0lock.c, but the lock has already
been granted or the transaction chosen as a victim in deadlock resolution. */
UNIV_INTERN
void
que_thr_stop_for_mysql(
/*===================*/
	que_thr_t*	thr)	/*!< in: query thread */
{
	trx_t*	trx;

	trx = thr_get_trx(thr);

	mutex_enter(&kernel_mutex);

	if (thr->state == QUE_THR_RUNNING) {

		if (trx->error_state != DB_SUCCESS
		    && trx->error_state != DB_LOCK_WAIT) {

			/* Error handling built for the MySQL interface */
			thr->state = QUE_THR_COMPLETED;
		} else {
			/* It must have been a lock wait but the lock was
			already released, or this transaction was chosen
			as a victim in selective deadlock resolution */

			mutex_exit(&kernel_mutex);

			return;
		}
	}

	ut_ad(thr->is_active == TRUE);
	ut_ad(trx->n_active_thrs == 1);
	ut_ad(thr->graph->n_active_thrs == 1);

	thr->is_active = FALSE;
	(thr->graph)->n_active_thrs--;

	trx->n_active_thrs--;

	mutex_exit(&kernel_mutex);
}

/**********************************************************************//**
Moves a thread from another state to the QUE_THR_RUNNING state. Increments
the n_active_thrs counters of the query graph and transaction if thr was
not active. */
UNIV_INTERN
void
que_thr_move_to_run_state_for_mysql(
/*================================*/
	que_thr_t*	thr,	/*!< in: an query thread */
	trx_t*		trx)	/*!< in: transaction */
{
	if (thr->magic_n != QUE_THR_MAGIC_N) {
		fprintf(stderr,
			"que_thr struct appears corrupt; magic n %lu\n",
			(unsigned long) thr->magic_n);

		mem_analyze_corruption(thr);

		ut_error;
	}

	if (!thr->is_active) {

		thr->graph->n_active_thrs++;

		trx->n_active_thrs++;

		thr->is_active = TRUE;
	}

	thr->state = QUE_THR_RUNNING;
}

/**********************************************************************//**
A patch for MySQL used to 'stop' a dummy query thread used in MySQL
select, when there is no error or lock wait. */
UNIV_INTERN
void
que_thr_stop_for_mysql_no_error(
/*============================*/
	que_thr_t*	thr,	/*!< in: query thread */
	trx_t*		trx)	/*!< in: transaction */
{
	ut_ad(thr->state == QUE_THR_RUNNING);
	ut_ad(thr->is_active == TRUE);
	ut_ad(trx->n_active_thrs == 1);
	ut_ad(thr->graph->n_active_thrs == 1);

	if (thr->magic_n != QUE_THR_MAGIC_N) {
		fprintf(stderr,
			"que_thr struct appears corrupt; magic n %lu\n",
			(unsigned long) thr->magic_n);

		mem_analyze_corruption(thr);

		ut_error;
	}

	thr->state = QUE_THR_COMPLETED;

	thr->is_active = FALSE;
	(thr->graph)->n_active_thrs--;

	trx->n_active_thrs--;
}

/****************************************************************//**
Get the first containing loop node (e.g. while_node_t or for_node_t) for the
given node, or NULL if the node is not within a loop.
@return	containing loop node, or NULL. */
UNIV_INTERN
que_node_t*
que_node_get_containing_loop_node(
/*==============================*/
	que_node_t*	node)	/*!< in: node */
{
	ut_ad(node);

	for (;;) {
		ulint	type;

		node = que_node_get_parent(node);

		if (!node) {
			break;
		}

		type = que_node_get_type(node);

		if ((type == QUE_NODE_FOR) || (type == QUE_NODE_WHILE)) {
			break;
		}
	}

	return(node);
}

/**********************************************************************//**
Prints info of an SQL query graph node. */
UNIV_INTERN
void
que_node_print_info(
/*================*/
	que_node_t*	node)	/*!< in: query graph node */
{
	ulint		type;
	const char*	str;

	type = que_node_get_type(node);

	if (type == QUE_NODE_SELECT) {
		str = "SELECT";
	} else if (type == QUE_NODE_INSERT) {
		str = "INSERT";
	} else if (type == QUE_NODE_UPDATE) {
		str = "UPDATE";
	} else if (type == QUE_NODE_WHILE) {
		str = "WHILE";
	} else if (type == QUE_NODE_ASSIGNMENT) {
		str = "ASSIGNMENT";
	} else if (type == QUE_NODE_IF) {
		str = "IF";
	} else if (type == QUE_NODE_FETCH) {
		str = "FETCH";
	} else if (type == QUE_NODE_OPEN) {
		str = "OPEN";
	} else if (type == QUE_NODE_PROC) {
		str = "STORED PROCEDURE";
	} else if (type == QUE_NODE_FUNC) {
		str = "FUNCTION";
	} else if (type == QUE_NODE_LOCK) {
		str = "LOCK";
	} else if (type == QUE_NODE_THR) {
		str = "QUERY THREAD";
	} else if (type == QUE_NODE_COMMIT) {
		str = "COMMIT";
	} else if (type == QUE_NODE_UNDO) {
		str = "UNDO ROW";
	} else if (type == QUE_NODE_PURGE) {
		str = "PURGE ROW";
	} else if (type == QUE_NODE_ROLLBACK) {
		str = "ROLLBACK";
	} else if (type == QUE_NODE_CREATE_TABLE) {
		str = "CREATE TABLE";
	} else if (type == QUE_NODE_CREATE_INDEX) {
		str = "CREATE INDEX";
	} else if (type == QUE_NODE_FOR) {
		str = "FOR LOOP";
	} else if (type == QUE_NODE_RETURN) {
		str = "RETURN";
	} else if (type == QUE_NODE_EXIT) {
		str = "EXIT";
	} else {
		//str = "UNKNOWN NODE TYPE";
	  if(type==5)str = "UNKNOWN NODE TYPE(:QUE_NODE_CURSOR)";
	  if(type==7)str = "UNKNOWN NODE TYPE(:QUE_NODE_AGGREGATE)";
	  if(type==8)str = "UNKNOWN NODE TYPE(:QUE_NODE_FORK)";
	  if(type==16)str = "UNKNOWN NODE TYPE(:QUE_NODE_SYMBOL)";
	  if(type==17)str = "UNKNOWN NODE TYPE(:QUE_NODE_RES_WORD)";
	  if(type==19)str = "UNKNOWN NODE TYPE(:QUE_NODE_ORDER)";
	  if(type==26)str = "UNKNOWN NODE TYPE(:QUE_NODE_COL_ASSIGNMENT)";
	  if(type==29)str = "UNKNOWN NODE TYPE(:QUE_NODE_QUE_NODE_ROW_PRINTF)";
	  if(type==30)str = "UNKNOWN NODE TYPE(:QUE_NODE_ELSIF)";
	  if(type==31)str = "UNKNOWN NODE TYPE(:QUE_NODE_CALL)"; //----2011-10-12-17-09;
	}

	fprintf(stderr, "Node type %lu: %s, address %p\n",
		(ulong) type, str, (void*) node);
}

#define UNIV_DEBUG
#define que_trace_on 1 //z+ //----2011-10-12-16-27;
/**********************************************************************//**
Performs an execution step on a query thread.
@return query thread to run next: it may differ from the input
parameter if, e.g., a subprocedure call is made */
UNIV_INLINE
que_thr_t*
que_thr_step(
/*=========*/
	que_thr_t*	thr)	/*!< in: query thread */
{
	que_node_t*	node;
	que_thr_t*	old_thr;
	trx_t*		trx;
	ulint		type;

	trx = thr_get_trx(thr);

	ut_ad(thr->state == QUE_THR_RUNNING);
	ut_a(trx->error_state == DB_SUCCESS);

	thr->resource++;

	node = thr->run_node; //“que_thr_init_command:“thr->run_node = thr; ..thr->prev_node = thr->common.parent; ...””/“p ((que_thr_t)node)->child”失败/可在pars_procedure_definition下断点查看thr和node的值+并与此处的node和que_thr_node_step的“thr-child”比较！！//----2011-10-12-17-33;--17-35;+--2011-10-12-19-45记录此；；;
	type = que_node_get_type(node);

	old_thr = thr;

#ifdef UNIV_DEBUG
	if (que_trace_on) {
		fputs("To execute: ", stderr);
		que_node_print_info(node);
	}
#endif
	if (type & QUE_NODE_CONTROL_STAT) {
		if ((thr->prev_node != que_node_get_parent(node))
		    && que_node_get_next(thr->prev_node)) {

			/* The control statements, like WHILE, always pass the
			control to the next child statement if there is any
			child left */

			thr->run_node = que_node_get_next(thr->prev_node);

		} else if (type == QUE_NODE_IF) {
			if_step(thr);
		} else if (type == QUE_NODE_FOR) {
			for_step(thr);
		} else if (type == QUE_NODE_PROC) {

			/* We can access trx->undo_no without reserving
			trx->undo_mutex, because there cannot be active query
			threads doing updating or inserting at the moment! */

			if (thr->prev_node == que_node_get_parent(node)) {
				trx->last_sql_stat_start.least_undo_no
					= trx->undo_no;
			}

			proc_step(thr); //=>(include/eval0proc.ic:)“thr->run_node = node->stat_list;”//----2011-10-12-19-56; //zlq //==>INSERT(/(row0ins.c:)“err = lock_table(0, node->table, LOCK_IX, thr);”)//----2011-10-12-20-03！！；;//(dict0mem.h:)“struct dict_table_struct”=>在“(row0ins.c:)row_ins_step”测试"p node->table->name"！！；----2011-10-12-20-05；+--2011-10-12-20-08记录此！/R;  //zlq
		} else if (type == QUE_NODE_WHILE) {
			while_step(thr);
		} else {
			ut_error;
		}
	} else if (type == QUE_NODE_ASSIGNMENT) {
		assign_step(thr);
	} else if (type == QUE_NODE_SELECT) {
		thr = row_sel_step(thr);
	} else if (type == QUE_NODE_INSERT) {
		thr = row_ins_step(thr);
	} else if (type == QUE_NODE_UPDATE) {
		thr = row_upd_step(thr);
	} else if (type == QUE_NODE_FETCH) {
		thr = fetch_step(thr);
	} else if (type == QUE_NODE_OPEN) {
		thr = open_step(thr);
	} else if (type == QUE_NODE_FUNC) {
		proc_eval_step(thr);

	} else if (type == QUE_NODE_LOCK) {

		ut_error;
		/*
		thr = que_lock_step(thr);
		*/
	} else if (type == QUE_NODE_THR) {
		thr = que_thr_node_step(thr);//=>“thr->run_node = thr->child;” //----2011-10-12-19-46;
	} else if (type == QUE_NODE_COMMIT) {
		thr = trx_commit_step(thr);
	} else if (type == QUE_NODE_UNDO) {
		thr = row_undo_step(thr);
	} else if (type == QUE_NODE_PURGE) {
		thr = row_purge_step(thr);
	} else if (type == QUE_NODE_RETURN) {
		thr = return_step(thr);
	} else if (type == QUE_NODE_EXIT) {
		thr = exit_step(thr);
	} else if (type == QUE_NODE_ROLLBACK) {
		thr = trx_rollback_step(thr);
	} else if (type == QUE_NODE_CREATE_TABLE) {
		thr = dict_create_table_step(thr);
	} else if (type == QUE_NODE_CREATE_INDEX) {
		thr = dict_create_index_step(thr);
	} else if (type == QUE_NODE_ROW_PRINTF) {
		thr = row_printf_step(thr);
	} else {
		ut_error;
	}

	if (type == QUE_NODE_EXIT) {
		old_thr->prev_node = que_node_get_containing_loop_node(node);
	} else {
		old_thr->prev_node = node;
	}

	if (thr) {
		ut_a(thr_get_trx(thr)->error_state == DB_SUCCESS);
	}

	return(thr);
}

/**********************************************************************//**
Run a query thread until it finishes or encounters e.g. a lock wait. */
static
void
que_run_threads_low(
/*================*/
	que_thr_t*	thr)	/*!< in: query thread */
{
	que_thr_t*	next_thr;
	ulint		loop_count;

	ut_ad(thr->state == QUE_THR_RUNNING);
	ut_a(thr_get_trx(thr)->error_state == DB_SUCCESS);
	ut_ad(!mutex_own(&kernel_mutex));

	loop_count = QUE_MAX_LOOPS_WITHOUT_CHECK;
loop:
	/* Check that there is enough space in the log to accommodate
	possible log entries by this query step; if the operation can touch
	more than about 4 pages, checks must be made also within the query
	step! */

	log_free_check();

	/* Perform the actual query step: note that the query thread
	may change if, e.g., a subprocedure call is made */

	/*-------------------------*/
	next_thr = que_thr_step(thr);
	/*-------------------------*/
/*
	} else if (type == QUE_NODE_SELECT) {
		thr = row_sel_step(thr);
	} else if (type == QUE_NODE_INSERT) {
		thr = row_ins_step(thr);
	} else if (type == QUE_NODE_UPDATE) {
		thr = row_upd_step(thr);
	} else if (type == QUE_NODE_FETCH) {
		thr = fetch_step(thr);
	} else if (type == QUE_NODE_OPEN) {
		thr = open_step(thr);
	} else if (type == QUE_NODE_FUNC) {
		proc_eval_step(thr);

	} else if (type == QUE_NODE_LOCK) {

		ut_error;
		/ *
		thr = que_lock_step(thr);
		* /
	} else if (type == QUE_NODE_THR) {
		thr = que_thr_node_step(thr);//=>“thr->run_node = thr->child;” //----2011-10-12-19-46;
	} else if (type == QUE_NODE_COMMIT) {
		thr = trx_commit_step(thr);
	} else if (type == QUE_NODE_UNDO) {
		thr = row_undo_step(thr);
	} else if (type == QUE_NODE_PURGE) {
		thr = row_purge_step(thr);
	} else if (type == QUE_NODE_RETURN) {
		thr = return_step(thr);
	} else if (type == QUE_NODE_EXIT) {
		thr = exit_step(thr);
	} else if (type == QUE_NODE_ROLLBACK) {
		thr = trx_rollback_step(thr);
	} else if (type == QUE_NODE_CREATE_TABLE) {
		thr = dict_create_table_step(thr);
	} else if (type == QUE_NODE_CREATE_INDEX) {
		thr = dict_create_index_step(thr);
	} else if (type == QUE_NODE_ROW_PRINTF) {
		thr = row_printf_step(thr);
...
	return(thr);
*/
//=>"QUE_NODE_PROC"不会返回”新的next_thr“

	ut_a(!next_thr || (thr_get_trx(next_thr)->error_state == DB_SUCCESS));

	loop_count++;

	if (next_thr != thr) {
		ut_a(next_thr == NULL);

		/* This can change next_thr to a non-NULL value if there was
		a lock wait that already completed. */
		que_thr_dec_refer_count(thr, &next_thr);

		if (next_thr == NULL) {

			return;
		}

		loop_count = QUE_MAX_LOOPS_WITHOUT_CHECK;

		thr = next_thr;
	}

	goto loop;
}

/**********************************************************************//**
Run a query thread. Handles lock waits. */
UNIV_INTERN
void
que_run_threads(
/*============*/
	que_thr_t*	thr)	/*!< in: query thread */
{
loop:
	ut_a(thr_get_trx(thr)->error_state == DB_SUCCESS);
	que_run_threads_low(thr);

	mutex_enter(&kernel_mutex);

	switch (thr->state) {

	case QUE_THR_RUNNING:
		/* There probably was a lock wait, but it already ended
		before we came here: continue running thr */

		mutex_exit(&kernel_mutex);

		goto loop;

	case QUE_THR_LOCK_WAIT:
		mutex_exit(&kernel_mutex);

		/* The ..._mysql_... function works also for InnoDB's
		internal threads. Let us wait that the lock wait ends. */

		srv_suspend_mysql_thread(thr);

		if (thr_get_trx(thr)->error_state != DB_SUCCESS) {
			/* thr was chosen as a deadlock victim or there was
			a lock wait timeout */

			que_thr_dec_refer_count(thr, NULL);

			return;
		}

		goto loop;

	case QUE_THR_COMPLETED:
	case QUE_THR_COMMAND_WAIT:
		/* Do nothing */
		break;

	default:
		ut_error;
	}

	mutex_exit(&kernel_mutex);
}

/*********************************************************************//**
Evaluate the given SQL.
@return	error code or DB_SUCCESS */
UNIV_INTERN
ulint
que_eval_sql(
/*=========*/
	pars_info_t*	info,	/*!< in: info struct, or NULL */
	const char*	sql,	/*!< in: SQL string */
	ibool		reserve_dict_mutex,
				/*!< in: if TRUE, acquire/release
				dict_sys->mutex around call to pars_sql. */
	trx_t*		trx)	/*!< in: trx */
{
	que_thr_t*	thr;
	que_t*		graph;

	ut_a(trx->error_state == DB_SUCCESS);

	if (reserve_dict_mutex) {
		mutex_enter(&dict_sys->mutex);
	}

	graph = pars_sql(info, sql);

	if (reserve_dict_mutex) {
		mutex_exit(&dict_sys->mutex);
	}

	ut_a(graph);

	graph->trx = trx;
	trx->graph = NULL;

	graph->fork_type = QUE_FORK_MYSQL_INTERFACE;

	ut_a(thr = que_fork_start_command(graph));//=>“thr = UT_LIST_GET_FIRST(fork->thrs); ... thr = UT_LIST_GET_NEXT(thrs, thr); ... return(thr);”
/*
	thr = UT_LIST_GET_FIRST(fork->thrs);

	/ * We make a single pass over the thr list within which we note which
	threads are ready to run. * /
	while (thr) {
		switch (thr->state) {
		case QUE_THR_COMMAND_WAIT:

			/ * We have to send the initial message to query thread
			to start it * /

			que_thr_init_command(thr); //->“thr->run_node = thr;..thr->prev_node = thr->common.parent; .. que_thr_move_to_run_state(thr);” //----2011-10-12-19-13; //que_thr_move_to_run_state:-->“thr->state = QUE_THR_RUNNING;”//----2011-10-12-19-15;

			return(thr);
....
*/
/*
void
que_thr_init_command(
/ *=================* /
	que_thr_t*	thr)	/ *!< in: query thread * /
{
	thr->run_node = thr;
	thr->prev_node = thr->common.parent;

	que_thr_move_to_run_state(thr);
}
*/
/*
”que_thr_move_to_run_state:-->“thr->state = QUE_THR_RUNNING;“
*/

	que_run_threads(thr);//=>que_run_threads_low==>que_thr_step  //----2011-10-12-19-23;
/*
que_run_threads_low(
/ *================* /
	que_thr_t*	thr)	/ *!< in: query thread * /
{
	que_thr_t*	next_thr;
	ulint		loop_count;

	ut_ad(thr->state == QUE_THR_RUNNING);
  ...
  ...
}
*/
/*
que_run_threads(
/ *============* /
	que_thr_t*	thr)	/ *!< in: query thread * /
{
loop:
	ut_a(thr_get_trx(thr)->error_state == DB_SUCCESS);
	que_run_threads_low(thr);

	mutex_enter(&kernel_mutex);

	switch (thr->state) {

	case QUE_THR_RUNNING:
		/ * There probably was a lock wait, but it already ended
		before we came here: continue running thr * /

		mutex_exit(&kernel_mutex);

		goto loop;
  ....
*/

	que_graph_free(graph);

	return(trx->error_state);
}
/*
//pars_sql=>:
(pars0sym.h://SQL parser symbol table)
/ ** Index of sym_node_struct::field_nos corresponding to the clustered index * /
#define	SYM_CLUST_FIELD_NO	0
/ ** Index of sym_node_struct::field_nos corresponding to a [secondary index] * /
#define	SYM_SEC_FIELD_NO	1
//-
/ ** Types of a [symbol table node] * /  //sym_node_struct.token_type //:表/列/索引//游标//存储过程/函数/变量/文字符号
enum sym_tab_entry {
	SYM_VAR = 91,		/ *!< declared parameter or local variable of a procedure * /
	SYM_IMPLICIT_VAR,	/ *!< storage for a intermediate result of a calculation * /
	SYM_LIT,		/ *!< literal * /
	SYM_TABLE,		/ *!< database table name * /
	SYM_COLUMN,		/ *!< database table name * /
	SYM_CURSOR,		/ *!< [named cursor]{A2.2.} * /
	SYM_PROCEDURE_NAME,	/ *!< stored procedure name * /
	SYM_INDEX,		/ *!< database index name * /
	SYM_FUNCTION		/ *!< user function name * /
};
/ ** Symbol table node * /
struct sym_node_struct{
	que_common_t			common;		/ *!< node type: QUE_NODE_SYMBOL * /
	/ * NOTE: if the data field in 'common.val' is not NULL and the symbol table node is not for a temporary column,
        the memory for the value has been allocated from dynamic memory and it should be freed when the symbol table is discarded * /

	/ * 'alias' and 'indirection' are almost the same, but not quite.
	'alias' always points to the primary instance of the variable,
        while 'indirection' does the same only if we should use the primary instance's values for the node's data.
        This is usually the case, but when initializing a cursor (e.g., "DECLARE CURSOR c IS SELECT * FROM t WHERE id = x;"),
        we copy the values from the primary instance to the cursor's instance so that they are fixed for the duration of the cursor, and set 'indirection' to NULL.
        If we did not, the value of 'x' could change between fetches and things would break horribly.

	TODO: It would be cleaner to make 'indirection' a boolean field and always use 'alias' to refer to the primary node. * /

	sym_node_t*			indirection;	/ *!< pointer to [another symbol table node] which contains [the value for this node],
                                                              NULL otherwise * /
	sym_node_t*			alias;		/ *!< pointer to [another symbol table node] for which [this node is an alias],
                                                              NULL otherwise * /
        //[
	UT_LIST_NODE_T(sym_node_t)	col_var_list;	/ *!< list of [table columns] or a list of [input variables] for [an explicit cursor{A2.1.}] * /
	ibool				copy_val;	/ *!< TRUE if [a column and its value] should be copied to dynamic memory [when fetched] * /
        //]
	ulint				field_nos[2];	/ *!< if a column, in the position SYM_CLUST_FIELD_NO is the [field number in the clustered index];
                                                                           in the position SYM_SEC_FIELD_NO the [field number in the non-clustered index] to use first;
                                                                            if not found from the index, then ULINT_UNDEFINED * /
	ibool				resolved;	/ * !< TRUE if [the meaning of [a variable or a column]] has been resolved;
                                                               for literals this is always TRUE * /
//[
//(pars0pars.c/pars_sql:)
//sym_node = UT_LIST_GET_FIRST(pars_sym_tab_global->sym_list);
//while (sym_node) {
//  ut_a(sym_node->resolved);
//  sym_node = UT_LIST_GET_NEXT(sym_list, sym_node);
//}
//=>:
// sym_tab_struct
//   sym_list: sym_node, sym_node, ...
//--
//]
        //[
	enum sym_tab_entry		token_type;	/ *!< type of the parsed token * /  //=>“//:表/列/索引//游标//存储过程/函数/变量/文字符号”
	const char*			name;		/ *!< name of an id * /
	ulint				name_len;	/ *!< id name length * /
        //]
        //[
	dict_table_t*			table;		/ *!< table definition if a table id or a column id * /
	ulint				col_no;		/ *!< column number if a column * /
        //]
	sel_buf_t*			prefetch_buf;	/ *!< NULL, or a buffer for cached column values for prefetched	rows * /
	sel_node_t*			cursor_def;	/ *!< cursor definition select node if a named cursor * /
	ulint				param_type;	/ *!< PARS_INPUT, PARS_OUTPUT, or PARS_NOT_PARAM if not a procedure parameter * /
	sym_tab_t*			sym_table;	/ *!< back pointer to the symbol table * /
        //
	UT_LIST_NODE_T(sym_node_t)	sym_list;	/ *!< list of symbol nodes * /
//(ut0lst.h:)
//#define UT_LIST_NODE_T(TYPE)\  //{A3.1.}
//struct {\
//	TYPE *	prev;	/ *!< pointer to the previous node,\
//			NULL if start of list * /\
//	TYPE *	next;	/ *!< pointer to next node, NULL if end of list * /\
//}\
};
//-
(pars0pars.h:)
/ ** A [predefined function] or operator node in a parsing tree; this construct is also used for some non-functions like the assignment ':=' * /
struct func_node_struct{
	que_common_t	common;	/ *!< type: QUE_NODE_FUNC * /
	int		func;	/ *!< token code of the [function name] * /
	ulint		class;	/ *!< class of the function * /
//z //class关键字在g++下会报错;
	que_node_t*	args;	/ *!< argument(s) of the function * /
	UT_LIST_NODE_T(func_node_t) cond_list;
				/ *!< list of comparison conditions;
                                       defined only for comparison operator nodes except, presently, for OPT_SCROLL_TYPE ones * /
	UT_LIST_NODE_T(func_node_t) func_node_list;
				/ *!< list of function nodes in a parsed query graph * /
};
//-
/ ** [Extra information] supplied [for pars_sql()]. * /
struct pars_info_struct {
	mem_heap_t*	heap;		/ *!< our own memory heap * /
	ib_vector_t*	funcs;		/ *!< user functions, or NUll(pars_user_func_t*) * /
	ib_vector_t*	bound_lits;	/ *!< bound literals, or NULL(pars_bound_lit_t*) * /
	ib_vector_t*	bound_ids;	/ *!< bound ids, or NULL(pars_bound_id_t*) * /
	ibool		graph_owns_us;	/ *!< if TRUE (which is the default), que_graph_free() will free us * /
};
(pars0sym.h:)
/ ** Symbol table * /
struct sym_tab_struct{  //zlq
	que_t*			query_graph;    / *!< query graph generated by the parser {A1.2.} * /
//(pars0pars.c/pars_sql:)“graph = pars_sym_tab_global->query_graph;..graph->sym_tab = pars_sym_tab_global;”
//=>本结构与query_graph相互引用;
	const char*		sql_string;     / *!< SQL string to parse {A1.1.} * /
	size_t			string_len;     / *!< SQL string length * /
        //
	int			next_char_pos;  / *!< [position of the [next character]] in sql_string to [give to the lexical analyzer] * /
	pars_info_t*		info;	        / *!< extra information, or NULL * /
        //
	sym_node_list_t		sym_list;       / *!< list of [symbol nodes] in the symbol table * /
//“pars0types.h:typedef UT_LIST_BASE_NODE_T(sym_node_t)	sym_node_list_t;”
//“pars0types.h:typedef struct sym_node_struct		sym_node_t;”
//(ut0lst.h:)
//#define UT_LIST_BASE_NODE_T(TYPE)\  //{A3.2.}
//struct {\
//	ulint	count;	/ *!< count of nodes in list * /\
//	TYPE *	start;	/ *!< pointer to list start, NULL if empty * /\
//	TYPE *	end;	/ *!< pointer to list end, NULL if empty * /\
//}\
	UT_LIST_BASE_NODE_T(func_node_t)
				func_node_list; / *!< list of [[function nodes] in the parsed query graph] * /
        //
	mem_heap_t*		heap;	        / *!< memory heap [from which] we can allocate space * /
};
//“grep pars_sym_tab_global ./ -r”
//“pars0pars.h:extern sym_tab_t*	pars_sym_tab_global;”



--


[
[root@server01 InnoSQL-5.5.8]# grep "pars_sym_tab_global->sql_string" ./ -r
./storage/innobase/pars/pars0pars.c:		fwrite(pars_sym_tab_global->sql_string
./storage/innobase/pars/pars0pars.c:	ut_memcpy(buf, pars_sym_tab_global->sql_string
./storage/innobase/pars/pars0pars.c:	pars_sym_tab_global->sql_string = mem_heap_dup(
[root@server01 InnoSQL-5.5.8]#
[root@server01 InnoSQL-5.5.8]# grep "pars_get_lex_chars" ./ -r
./storage/innobase/pars/pars0pars.c:pars_get_lex_chars(
./storage/innobase/pars/pars0lex.l:#define YY_INPUT(buf, result, max_size) pars_get_lex_chars(buf, &result, max_size)
./storage/innobase/pars/lexyy.c:#define YY_INPUT(buf, result, max_size) pars_get_lex_chars(buf, &result, max_size)
./storage/innobase/include/pars0pars.h:pars_get_lex_chars(
[root@server01 InnoSQL-5.5.8]#
[root@server01 InnoSQL-5.5.8]# grep "YY_INPUT" ./ -r
./storage/innobase/pars/pars0lex.l:#define YY_INPUT(buf, result, max_size) pars_get_lex_chars(buf, &result, max_size)
./storage/innobase/pars/lexyy.c:#define YY_INPUT(buf, result, max_size) pars_get_lex_chars(buf, &result, max_size)
./storage/innobase/pars/lexyy.c:#ifndef YY_INPUT
./storage/innobase/pars/lexyy.c:#define YY_INPUT(buf,result,max_size) \
./storage/innobase/pars/lexyy.c:		YY_INPUT( (&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),
./storage/innobase/pars/lexyy.c:/ ** Discard all buffered characters. On the next scan, YY_INPUT will be called.
[root@server01 InnoSQL-5.5.8]#
./storage/innobase/pars/lexyy.c:			yyin = stdin;
./storage/innobase/pars/lexyy.c:        yyin = in_str ;
[root@server01 InnoSQL-5.5.8]#
[root@server01 InnoSQL-5.5.8]# grep "yy_get_next_buffer" ./ -r
./storage/innobase/pars/lexyy.c:static int yy_get_next_buffer (void );
./storage/innobase/pars/lexyy.c:		else switch ( yy_get_next_buffer(  ) )
./storage/innobase/pars/lexyy.c:					 * yy_get_next_buffer() to have set up
./storage/innobase/pars/lexyy.c:/ * yy_get_next_buffer - try to read in a new buffer
./storage/innobase/pars/lexyy.c:static int yy_get_next_buffer (void)
./storage/innobase/pars/lexyy.c:			switch ( yy_get_next_buffer(  ) )
./storage/innobase/pars/lexyy.c:     * called from yyrestart() or through yy_get_next_buffer.
[root@server01 InnoSQL-5.5.8]#
=>:
“ut_memcpy(buf, pars_sym_tab_global->sql_string + pars_sym_tab_global->next_char_pos, len);”
 <- pars_get_lex_chars <- YY_INPUT <- yy_get_next_buffer <- YY_DECL
                                                         <- input/yy_input //“#ifndef YY_NO_INPUT ... input/yy_input ... #endif”
(lexyy.c:)“#define YY_DECL UNIV_INTERN int yylex (void)”
(pars0grm.c:)
 #ifdef YYLEX_PARAM
 # define YYLEX yylex (YYLEX_PARAM)
 #else
 # define YYLEX yylex ()
 #endif
==>
(pars0grm.c:)“      yychar = YYLEX;”
]//----2011-10-12-14-20;


[
(gdb) s
To execute: Node type 9: QUERY THREAD, address 0x90f0818
To execute: Node type 1044: STORED PROCEDURE, address 0x90f0880
To execute: Node type 14: CREATE TABLE, address 0x90f0670
To execute: Node type 2: INSERT, address 0x90f06b0
To execute: Node type 9: QUERY THREAD, address 0x90e6c50
To execute: Node type 13: PURGE ROW, address 0x90e6cb8
To execute: Node type 9: QUERY THREAD, address 0x90e6c50
To execute: Node type 9: QUERY THREAD, address 0x90e6c50
To execute: Node type 13: PURGE ROW, address 0x90e6cb8
To execute: Node type 9: QUERY THREAD, address 0x90e6c50
]//----2011-10-12-16-36;
=>que0que.c:1344->1291
//----2011-10-12-16-41;
parse后执行“(gdb) call que_node_print_info(node)”
que_thr_step中执行“(gdb) call que_node_print_info(node)”
que_thr_node_step中执行“(gdb) call que_node_print_info(node)”
//----2011-10-12-16-49;--16-50;
--


[
que_eval_sql:“thr = que_fork_start_command(graph)”  //que_fork_struct=>que_thr_struct
 ->que_fork_start_command:“thr = UT_LIST_GET_FIRST(fork->thrs); ... que_thr_init_command(thr);”
 =>que_run_threads(thr) -> que_run_threads_low(thr) -> que_thr_step(thr)(/“thr = que_thr_node_step(thr);”)
<=(thr如何得到？)//(17-40)----2011-10-12-18-09;--18-17:记录此+检查分析过程+备份/R;
[
[root@server01 InnoSQL-5.5.8]# grep ">thrs" ./ -r
./storage/innobase/que/que0que.c:	UT_LIST_INIT(fork->thrs);
./storage/innobase/que/que0que.c:	UT_LIST_ADD_LAST(thrs, parent->thrs, thr); //(que0que.c:)“UT_LIST_ADD_LAST(thrs, parent->thrs, thr);”
./storage/innobase/que/que0que.c:	thr = UT_LIST_GET_FIRST(fork->thrs);
./storage/innobase/que/que0que.c:	thr = UT_LIST_GET_FIRST(fork->thrs);
./storage/innobase/que/que0que.c:	thr = UT_LIST_GET_FIRST(fork->thrs);
./storage/innobase/que/que0que.c:	thr_node = UT_LIST_GET_FIRST(fork->thrs);
./storage/innobase/que/que0que.c:		thr = UT_LIST_GET_FIRST(fork->thrs);
./storage/innobase/include/que0que.ic:	return(UT_LIST_GET_FIRST(fork->thrs));
./storage/innobase/include/que0que.ic:	thr = UT_LIST_GET_FIRST(fork->thrs);
[root@server01 InnoSQL-5.5.8]# grep "que_thr_create" ./ -r
./storage/innobase/que/que0que.c:que_thr_create(
./storage/innobase/trx/trx0purge.c:	thr = que_thr_create(fork, heap);
./storage/innobase/trx/trx0purge.c:	/ 	thr2 = que_thr_create(fork, fork, heap);
./storage/innobase/trx/trx0roll.c:	thr = que_thr_create(fork, heap);
./storage/innobase/trx/trx0roll.c:	thr = que_thr_create(fork, heap);
./storage/innobase/trx/trx0roll.c:	/ 	thr2 = que_thr_create(fork, heap); * /
./storage/innobase/pars/pars0pars.c:	thr = que_thr_create(fork, heap); //(pars0pars.c:)pars_procedure_definition:“thr = que_thr_create(fork, heap);” //zlq
./storage/innobase/pars/pars0pars.c:	thr = que_thr_create(fork, heap);
./storage/innobase/include/que0que.h:que_thr_create(
/----2011-10-12-17-20--17-30;+--17-40记录此！；;
--
(ut0lst.h:)
#define UT_LIST_INIT(BASE)\
{\
	(BASE).count = 0;\
	(BASE).start = NULL;\
	(BASE).end   = NULL;\
}\
...
#define UT_LIST_GET_FIRST(BASE)\
	(BASE).start
]
--
(que0que.c:)pars_procedure_definition:/1721
 graph-------fork--(含有)--thrs
 (fork赋给thr)  |        (thr追加到fork)
 graph---------thr--------thrs  //“thr = que_thr_create(fork, heap);”
                 |(child)
                 |
                 |(parent)
                 proc_node_struct
 “pars_sym_tab_global->query_graph = fork;”  //zlq
/----2011-10-12-19-00;+--19-06:记录此！；;
=> que_thr_step->que_thr_node_step /“que_eval_sql:“thr = que_fork_start_command(graph)”  //que_fork_struct=>que_thr_struct”
/----2011-10-12-19-10;
//
(2011-10-12-20-08--)--:
dict_create_table_step->("tab def"->"col def")->row_sel_step
=>dict0crea.c:1254 ==>SYS_TABLES/SYS_COLUMNS/SYS_INDEXES/SYS_FIELDS
(que0que.c:)“trx->error_state”返回11而不是10/(dict0crea.c:1385)“//error=DB_SUCCESS; //z+ //zlqlxm //--2011-10-09-21-30:OK;+--21-34:R; //-2011-10-12-20-14去除此句！！！；；;”
 db0error.h
--2011-10-12-20-31;
innodb_test1.c:39那句导致18号错: =>DB_DUPLICATE_KEY
==>在dict0crea.c中注释掉那句que_eval_sql: =>建成功SYS_FOREIGN表
--2011-10-12-20-49;
建index就出错=>即使建立非cluster索引还是错！；
--2011-10-12-20-52;
===>(dict0crea.c:1113)“if (node->state == INDEX_ADD_TO_CACHE) { ....” /建立索引要3步：INDEX_BUILD_INDEX_DEF,INDEX_BUILD_FIELD_DEF, INDEX_ADD_TO_CACHE
 “//			>= DICT_TF_FORMAT_ZIP);  //z- //因为trx_is_strict！/为什么ha_innodb__.cc中的stub无效呢？；----2011-09-20-10-02;”
--2011-10-12-21-00发现此问题--21-05再次检查！！！；；；;
(21-05)----2011-10-12-21-22记录此段！！；/R;
//
---;

x
*/


#include "sched.h"

/*
	Este algoritmo de planificacion consiste simplemente en que los procesos se van 
	ejecutando en el orden en el que han ido llegando a la runqueue.
	Si algun proceso sufre una operacion E/S, entonces pasa al final de la cola.
*/


static task_t* pick_next_task_fcfs(runqueue_t* rq)
{
	task_t* t=head_slist(&rq->tasks);
	if (t) remove_slist(&rq->tasks,t); //Ya hemos cogido la tarea de la lista; la eliminamos.
	return t;
}

static void enqueue_task_fcfs(task_t* t,runqueue_t* rq, int preempted)
{
	//Si la tarea ya esta en la runqueue o es vacia, no la añadimos //TODO PREGUNTAR COMO PODRIA DARSE ESTO
	if (t->on_rq || is_idle_task(t))
		return;

	//Tanto como si la tarea es nueva como si estaba bloqueada por E/S, la tenemos que insertar al final
	insert_slist(&rq->tasks,t); 
}

static task_t* steal_task_fcfs(runqueue_t* rq)
{

	//Cuando una cola esta muy poco cargada, trata de coger procesos de otra cola que este muy cargada
	//Por ello, coge el ultimo proceso de esa cola, y lo borra de ella.

	task_t* t=tail_slist(&rq->tasks);

	if (t) 
		remove_slist(&rq->tasks,t);

	return t;
}

sched_class_t fcfs_sched= {
	.pick_next_task=pick_next_task_fcfs,
	.enqueue_task=enqueue_task_fcfs,
	.steal_task=steal_task_fcfs
};
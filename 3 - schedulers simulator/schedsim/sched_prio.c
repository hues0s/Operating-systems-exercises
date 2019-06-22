#include "sched.h"

/*
	Este algoritmo consiste en ir cediendo la cpu a los procesos en funcion de su prioriodad.
	Si llega un proceso de mayor prioridad que el que esta actualmente en la CPU, se le cede
	la posicion. Si en cambio el proceso que llega tiene menor prioridad, se añade a la cola
	siempre en orden. 

	ORDEN: siempre de menor a mayor, siendo 1 la maxima prioridad.

	En caso de migración, se cederá la tarea menos prioritaria de las que estén en la cola,
	que teoricamente deberia ser la que este al final de la cola.
*/

static task_t* pick_next_task_prio(runqueue_t* rq)
{
	task_t* t=head_slist(&rq->tasks);
	if (t) remove_slist(&rq->tasks,t);
	return t;
}

static int compare_tasks_cpu_priority(void *t1,void *t2)
{
	task_t* tsk1=(task_t*)t1;
	task_t* tsk2=(task_t*)t2;

	return tsk1->prio-tsk2->prio;
}

static void enqueue_task_prio(task_t* t,runqueue_t* rq, int preempted)
{
	/*
		Esta funcion es muy importante, ya que para que todo funcione, 
		se deben insertar en la cola en orden de prioridad las tareas
		que vayan llegando, para que luego el resto de tareas sean muy simples.
	*/

	//Si la tarea ya esta en la runqueue o es vacia, no la añadimos.
	if (t->on_rq || is_idle_task(t)) return;

	//Añadimos en orden la tarea en la runqueue
	//sorted_insert_slist(&rq->tasks, t, 1, compare_tasks_cpu_priority); 

	if (t->flags & TF_INSERT_FRONT) {
		//Clear flag
		t->flags&=~TF_INSERT_FRONT;
		sorted_insert_slist_front(&rq->tasks, t, 1, compare_tasks_cpu_priority);  //Push task
	} else
		sorted_insert_slist(&rq->tasks, t, 1, compare_tasks_cpu_priority);  //Push task


	//Si la tarea no ha sido echada (preempted) de la CPU y
	//si la prioridad de la tarea recien añadida es menor que la de la CPU, sacamos la de la CPU.
	if(!preempted && t->prio < rq->cur_task->prio){
		rq->need_resched=TRUE;
		rq->cur_task->flags|=TF_INSERT_FRONT;
	}

}


static task_t* steal_task_prio(runqueue_t* rq)
{
	/*
		Como la cola esta ordenada de mayor a menor prioridad, 
		simplemente cogemos la del final de la cola ya que sera la de menor prioridad.
	*/
	task_t* t=tail_slist(&rq->tasks);
	if (t) remove_slist(&rq->tasks,t);
	return t;
}


sched_class_t prio_sched= {
	.pick_next_task=pick_next_task_prio,
	.enqueue_task=enqueue_task_prio,
	.steal_task=steal_task_prio
};
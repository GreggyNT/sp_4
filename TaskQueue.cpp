#include "TaskQueue.h"

TaskQueue::TaskQueue()
{
	InitializeCriticalSectionAndSpinCount(&csection, SPINS);
	first = NULL;
	last = NULL;
}

Task* TaskQueue::Front()
{
	EnterCriticalSection(&csection);
	if (Empty())
	{
		LeaveCriticalSection(&csection);
		return NULL;
	}
	if (first == last)
	{
		Node* tmpReturn = first;
		first = NULL;
		last = NULL;
		LeaveCriticalSection(&csection);
		return tmpReturn->data;
	}
	Node* tmpFirst = first;
	first = first->next;
	first->prev = NULL;

	LeaveCriticalSection(&csection);
	Task* task = tmpFirst->data;
	delete(tmpFirst);
	return task;

}

void TaskQueue::Enqueue(Task* indata)
{
	Node* tmp = new Node();
	tmp->data = indata;
	tmp->next = NULL;
	tmp->prev = NULL;

	EnterCriticalSection(&csection);
	if (Empty())
	{
		first = tmp;
		last = tmp;
	}
	else
	{
		Node* tmpLast = last;
		last->next = tmp;
		tmp->prev = tmpLast;
		last = tmp;

	}
	LeaveCriticalSection(&csection);
}

bool TaskQueue::Empty()
{
	EnterCriticalSection(&csection);
	bool isEmpty = (first == NULL) && (last == NULL);
	LeaveCriticalSection(&csection);
	return isEmpty;
}

TaskQueue::~TaskQueue()
{
	DeleteCriticalSection(&csection);
}

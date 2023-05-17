#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

struct Node
{
	Node(int value) : _value(value), _next(nullptr), _node_mutex(new mutex) {}
	~Node() { delete _node_mutex; }
	int _value;
	Node* _next;
	mutex* _node_mutex;
};

class FineGrainedQueue
{
public:
	FineGrainedQueue() : _head(nullptr), _queue_mutex(new mutex) {}
	~FineGrainedQueue() { delete _queue_mutex; }

	void push_back(int value)
	{
		Node* newNode = new Node(value);
		if (_head == nullptr)
		{
			_head = newNode;
			return;
		}
		Node* last = _head;
		while (last->_next != nullptr)
		{
			last = last->_next;
		}
		last->_next = newNode;
	}

	void show()
	{
		Node* current = _head;
		if (isEmpty())
		{
			cout << "Queue is empty" << endl;
		}
		while (current != nullptr)
		{
			cout << current->_value << " ";
			current = current->_next;
		}
	}
	
	bool isEmpty()
	{
		return _head == nullptr;
	}

	void insertIntoMiddle(int value, int pos)
	{
		Node* newNode = new Node(value);
		_queue_mutex->lock();
		Node* current = _head;
		current->_node_mutex->lock();
		_queue_mutex->unlock();

		int currentPos = 0;

		while (currentPos < pos - 2 && current->_next)
		{
			Node* previous = current;
			current->_next->_node_mutex->lock();
			current = current->_next;
			previous->_node_mutex->unlock();
			currentPos++;
		}

		Node* nextNode = current->_next;
		current->_next = newNode;
		current->_node_mutex->unlock();
		newNode->_next = nextNode;

		lock_guard<mutex>lock(*_queue_mutex);
		cout << "\nThread ID" << this_thread::get_id << "\t";
		this->show();
	}

	void remove(int value)
	{
		Node* previous = _head;
		Node* current = _head->_next;
		_queue_mutex->lock();

		if (isEmpty())
		{
			_queue_mutex->unlock();
			return;
		}

		previous->_node_mutex->lock();
		if (current)
			current->_node_mutex->lock();
		{
			_head = current;
			_head->_next = current->_next;
			previous->_node_mutex->unlock();
			current->_node_mutex->unlock();
			_queue_mutex->unlock();
			delete previous;
			return;
		}

		_queue_mutex->unlock();

		while (current)
		{
			if (current->_value == value)
			{
				previous->_next = current->_next;
				previous->_node_mutex->unlock();
				current->_node_mutex->unlock();
				delete current;
				return;
			}
			Node* old_prev = previous;
			previous = current;
			current = current->_next;
			old_prev->_node_mutex->unlock();
			if (current)
				current->_node_mutex->lock();
		}
		previous->_node_mutex->unlock();
	}

	private:
		Node* _head;
		mutex* _queue_mutex;
};

int main()
{
	FineGrainedQueue FGQ;

	int size = 15;
	for (size_t i = 0; i <= size; i++)
	{
		FGQ.push_back(2 * i + 1);
	}
	cout << "Create FGQ: ";
	FGQ.show();

	cout << "\nFGQ after insertion in the middle: ";
	thread t1(&FineGrainedQueue::insertIntoMiddle, &FGQ, 100, 3);
	thread t2(&FineGrainedQueue::insertIntoMiddle, &FGQ, 200, 7);
	thread t3(&FineGrainedQueue::insertIntoMiddle, &FGQ, 300, 8);
	if (t1.joinable())
		t1.join();
	if (t2.joinable())
		t2.join();
	if (t3.joinable())
		t3.join();

	thread t4(&FineGrainedQueue::remove, &FGQ, 31);
	if (t4.joinable())
		t4.join();
	cout << "\nFGQ after remove: ";
	FGQ.show();

	thread t5(&FineGrainedQueue::remove, &FGQ, 7);
	if (t5.joinable())
		t5.join();
	cout << "\nFGQ after remove first element: ";
	FGQ.show();
	return 0;

}
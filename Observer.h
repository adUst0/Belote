#pragma once
#include <vector>
#include "Player.h"
#include "Card.h"
#include "Utils.h"

template <typename Data>
class Subject;

template <typename Data>
class Observer
{
	friend Subject<Data>;
public:
	virtual ~Observer();
	virtual void notify(const Data& data) = 0;

private:
	std::vector<Subject<Data>*> m_subjects;
};


template <typename Data>
class Subject
{
	friend Observer<Data>;
public:
	virtual ~Subject();

	void addObserver(Observer<Data>& observer);
	void removeObserver(Observer<Data>& observer);
	void notifyObservers(const Data& data);

private:
	std::vector<Observer<Data>*> m_observers;
};

template <typename Data>
Observer<Data>::~Observer<Data>()
{
	for (Subject<Data>* subject : m_subjects)
	{
		std::erase(subject->m_observers, this);
	}
}

template <typename Data>
void Subject<Data>::addObserver(Observer<Data>& observer)
{
	Utils::emplace_back_unique(m_observers, &observer);
	Utils::emplace_back_unique(observer.m_subjects, this);
}

template <typename Data>
void Subject<Data>::removeObserver(Observer<Data>& observer)
{
	std::erase(m_observers, &observer);
}

template <typename Data>
void Subject<Data>::notifyObservers(const Data& data)
{
	for (Observer<Data>* observer : m_observers)
	{
		observer->notify(data);
	}
}

template <typename Data>
Subject<Data>::~Subject<Data>()
{
	for (Observer<Data>* observer : m_observers)
	{
		std::erase(observer->m_subjects, this);
	}
}

// TODO: move to another file
struct NotifyCardDealing
{
	const Player& m_player;
	const Card& m_card;
};
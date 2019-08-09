#include <iostream>
#include <vector>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
using namespace std;


static int totalRequestGroup1;
static int totalRequestGroup2;
static int waitGroup;
static int waitPosition;
int startingGroup;
int remainingRequestsOfFirstGroup;

static pthread_mutex_t mutex;
static pthread_mutex_t group_mutex;
static pthread_cond_t positionsCond[10];
static pthread_cond_t group_cond;
static int positions[10];

struct argument
{
	int id;
	int group;
	int position;
	int extraTime;
	int requestTime;
};

void* user_action(void* arg)
{
	argument* user = (argument*) arg;

	if (user->group == 1)
		totalRequestGroup1++;
	else
		totalRequestGroup2++;
	cout << "User " << user->id << " from Group " << user->group << " arrives to the DBMS\n";
	

	pthread_mutex_lock(&group_mutex);
	if (user->group != startingGroup)
	{
		waitGroup++;
		cout << "User " << user->id << " is waiting due to its group\n";
		pthread_cond_wait(&group_cond, &group_mutex);
	}
	pthread_mutex_unlock(&group_mutex);
	
	pthread_mutex_lock(&mutex);
	if (positions[user->position - 1] != 0)
	{
		waitPosition++;
		cout << "User " << user->id << " is waiting : position " << user->position << " of the database is being used by user " << positions[user->position - 1] << endl;
		pthread_cond_wait(&positionsCond[user->position - 1], &mutex);
	}
	positions[user->position - 1] = user->id;
	cout << "User " << user->id << " is accessing the position " << user->position << " of the database for " << user->requestTime << " second(s)\n";
	pthread_mutex_unlock(&mutex);

	sleep(user->requestTime);

	pthread_mutex_lock(&mutex);
	cout << "User " << user->id << " finished its execution\n";
	positions[user->position - 1] = 0;
	pthread_cond_signal(&positionsCond[user->position - 1]);
	pthread_mutex_unlock(&mutex);

	pthread_mutex_lock(&group_mutex);
	remainingRequestsOfFirstGroup--;
	if (remainingRequestsOfFirstGroup == 0)
	{
		int nextGroup;
		if (startingGroup == 1)
			nextGroup = 2;
		else
			nextGroup = 1;
		cout << "\nAll users from Group " << startingGroup << " finished their execution\n";
		cout << "The users from Group " << nextGroup << " start their execution\n\n";
		startingGroup = nextGroup;
		pthread_cond_broadcast(&group_cond);
	}
	pthread_mutex_unlock(&group_mutex);
	
	return NULL;
}

int main()
{
	remainingRequestsOfFirstGroup = 0;
	totalRequestGroup1 = 0;
	totalRequestGroup2 = 0;
	waitGroup = 0;
	waitPosition = 0;
	
	cin >> startingGroup;
	int group;
	int count = 0;
	vector<argument> user;
	while (cin >> group)
	{
		if (group == startingGroup)
			remainingRequestsOfFirstGroup++;

		argument temp;
		temp.group = group;
		count++;
		temp.id = count;
		cin >> temp.position;
		cin >> temp.extraTime;
		cin >> temp.requestTime;
		user.push_back(temp);
		
	}
	pthread_t tids[count];
	for (int i = 0; i < count; i++)
	{
		sleep(user[i].extraTime);
		pthread_create(&tids[i], NULL, user_action, &user[i]);
		//sleep(1);
	}
	for (int i = 0; i < count; i++)
	{
		pthread_join(tids[i], NULL);
	}

	cout << "\nTotal Requests:\n";
	cout << "	Groups 1: " << totalRequestGroup1 << endl;
	cout << "	Groups 2: " << totalRequestGroup2 << endl;
	cout << endl;
	cout << "Requests that waited:\n";
	cout << "	Due to its group: " << waitGroup << endl;
	cout << "	Due to a locked position: " << waitPosition << endl;
	return 0;
}
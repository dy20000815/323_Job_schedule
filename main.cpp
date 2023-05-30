#include "iostream"
#include "fstream"

using namespace std;

class node {
public:
	int jobID;
	int jobTime;
	node* next;

	node(int id, int time) {
		jobID = id;
		jobTime = time;
		next = NULL;
	}
};

class schedule {
public:
	int numNodes;
	int numProcs;
	int procUsed;
	int currentTime;
	int totalJobTimes;
	int* jobTimeAry;
	int* jobStatus;
	int** Matrix;
	int** Table;
	node* OPEN;

	schedule(int nn, int np) {
		numNodes = nn;
		numProcs = np;
		procUsed = 0;
		currentTime = 0;
		totalJobTimes = 0;
		jobTimeAry = new int[numNodes + 1];
		jobStatus = new int[numNodes + 1];
		for (int i = 1; i < numNodes + 1; i++) {
			jobStatus[i] = 1;
			jobTimeAry[i] = 0;
		}Matrix = new int* [numNodes + 1];
		for (int j = 0; j <= numNodes; j++) {
			Matrix[j] = new int[numNodes + 1];
			for (int i = 0; i <= numNodes; i++) {
				Matrix[j][i] = 0;
			}
		}Matrix[0][0] = numNodes;
		OPEN = new node(99, 99);
	}

	void loadMatrix(ifstream& input) {
		int row, col;
		while (input.eof() == false) {
			input >> row >> col;
			Matrix[row][col] = 1;
		}
	}

	void loadJobTimeAry(ifstream& input) {
		while (input.eof() == false) {
			int id;
			int time;
			input >> id >> time;
			jobTimeAry[id] = time;
			totalJobTimes += time;
		}
		Table = new int* [numProcs + 1];
		for (int j = 0; j < numProcs + 1; j++) {
			Table[j] = new int[totalJobTimes + 1];
			for (int i = 0; i <= totalJobTimes; i++) {
				Table[j][i] = 0;
			}
		}
	}

	void setMatrix() {
		for (int i = 1; i <= numNodes; i++) {
			for (int j = 1; j <= numNodes; j++) {
				if (Matrix[i][j] != 0) {
					Matrix[0][j]++;
					Matrix[i][0]++;
				}
			}
		}
	}

	void printMatrix(ofstream& output) {
		output << "printing matrix: " << endl << "\t";
		for (int i = 0; i <= numNodes; i++) {
			output << "\tcol " << i << "\t";
		}output << endl;
		for (int i = 0; i <= numNodes; i++) {
			output << "row " << i << ":\t";
			for (int j = 0; j <= numNodes; j++) {
				output << Matrix[i][j] << "           ";
			}output << endl;
		}
	}

	int findOrphan() {
		for (int i = 1; i <= numNodes; i++) {
			if (Matrix[0][i] == 0 && jobStatus[i] == 1) {
				jobStatus[i] = 2;
				return i;
			}
		}
		return -1;
	}

	void OpenInsert(node* n) {
		node* curr = OPEN;
		while (curr->next != NULL && Matrix[n->jobID][0] < Matrix[curr->next->jobID][0]) {
			curr = curr->next;
		}n->next = curr->next;
		curr->next = n;
	}

	void printOPEN(ofstream& output) {
		output << "printing OPEN:" << endl;
		node* curr = OPEN;
		while (curr->next != NULL) {
			output <<"("<< curr->next->jobID << ","<<curr->next->jobTime<< " ) -> ";
			curr = curr->next;
		}output << "end"<< endl;
	}

	int getNextProc() {
		for (int i = 1; i <= numProcs; i++) {
			if (Table[i][currentTime] == 0) {
				return i;
			}
		}return -1;
	}

	void fillOPEN(ofstream& output) {
		while (true) {
			int jid = findOrphan();
			if (jid <= 0) {
				break;
			}
			else {
				node* nnode = new node(jid, jobTimeAry[jid]);
				OpenInsert(nnode);
				printOPEN(output);
			}
		}
	}

	void fillTable() {
		while ( OPEN->next != NULL && procUsed <= numProcs) {
			int availProc = getNextProc();
			if (availProc >= 0) {
				node* newJob = OPEN->next;
				OPEN->next = OPEN->next->next;
				putJobOnTable(availProc, newJob->jobID, newJob->jobTime);
				if (availProc > procUsed) {
					procUsed++;
				}
			}
			else break;
		}
	}

	void putJobOnTable(int ap, int jid, int jtime) {
		int time = currentTime;
		int endTime = time + jtime;
		while (time < endTime) {
			Table[ap][time] = jid;
			time++;
		}
	}

	void printTable(ofstream& output) {
		output << "ProcUsed: " << procUsed << "\tcurrentTime: " << currentTime << endl;
		output << "Time: \t\t";
		for (int i = 0; i <= totalJobTimes; i++) {
			output << "|\t" << i << "\t";
		}output << endl;
		for (int j = 1; j <= numProcs; j++) {
			output << "proc: " << j << " \t";
			for (int k = 0; k <= totalJobTimes; k++) {
				output << "|\t" << Table[j][k] << "\t";
			}output << endl;
		}output << endl;
	}

	bool checkCycle() {
		if (OPEN->next == NULL && isGraphEmpty() == false && checkProcs() == true && checkOrphan()==false) {
			return true;
		}
		else return false;
	}

	bool checkProcs() {
		for (int i = 1; i <= numProcs; i++) {
			if (Table[i][currentTime] != 0) {
				return false;
			}
		}return true;
	}

	bool checkOrphan() {
		for (int i = 1; i <= numNodes; i++) {
			if (Matrix[0][i] == 0 && jobStatus[i] == 1) {
				return true;
			}
		}
		return false;
	}

	bool isGraphEmpty() {
		if (Matrix[0][0] == 0) {
			return true;
		}
		else return false;
	}

	void deleteJob(int i) {
		jobStatus[i] = 0;
		Matrix[0][0]--;
		for (int j = 1; j <= numNodes; j++) {
			if (Matrix[i][j] > 0) {
				Matrix[0][j]--;
			}
		}
	}

	void deleteDoneJob(ofstream& output) {
		int proc = 1;
		while (proc <= procUsed) {
			if (Table[proc][currentTime] <= 0 && Table[proc][currentTime - 1] > 0) {
				int jid = Table[proc][currentTime - 1];
				deleteJob(jid);
			}printMatrix(output);
			proc++;
		}
		
	}
};

int main(int argc, char* argv[]) {
	ifstream input1, input2;
	input1.open(argv[1]);
	input2.open(argv[2]);
	int nnode1, nnode2;
	input1 >> nnode1;
	input2 >> nnode2; 
	if (nnode1 != nnode2) {
		cout << "2 files are not matched." << endl;
	}
	int np = atoi(argv[3]);
	if (np <= 0) {
		cout << "need 1 or more processors." << endl;
		exit(-1);
	}
	else if (np > nnode1) {
		np = nnode1;
	}
	ofstream output, debug;
	output.open(argv[4]);
	debug.open(argv[5]);
	schedule* sch = new schedule(nnode1, np);
	sch->loadMatrix(input1);
	sch->setMatrix();
	sch->printMatrix(debug);
	sch->loadJobTimeAry(input2);
	sch->printTable(debug);
	while (sch->isGraphEmpty() == false) {
		sch->fillOPEN(debug);
		sch->printOPEN(debug);
		sch->fillTable();
		sch->printTable(debug);
		sch->currentTime++;
		sch->deleteDoneJob(debug);
		if (sch->checkCycle() == true) {
			output << "there is a cycle in the graph!!!" << endl;
			return 0;
		}
	}
	sch->printTable(output);
	input1.close();
	input2.close();
	output.close();
	debug.close();
}

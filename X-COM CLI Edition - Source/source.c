#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>


struct soldier { // 각 병사를 나타내는 구조체
	char name; // 병사의 이름
	int maxHP; // 병사의 최대 HP
	int curHP; // 병사의 현재 HP
	int aim; // 병사의 조준 능력치
	int ATK; // 병사의 화력 능력치
	int DEX; // 병사의 민첩 능력치
	int weak; // 병사의 주파수 대역
	int location[2]; // 병사의 위치 - x축과 y축
	int live; // 병사가 살아있는지의 여부를 0과 1로 표현
	char team; // 병사의 소속 팀을 나타냄
	int magStack[5]; // 병사의 남은 장탄 수
	int stackTop; // 병사의 탄창 상태 구현에 사용
};
struct soldierQ { // 병사의 순서 구현에 사용하는 구조체
	char name; // 병사의 이름
	struct soldierQ *next; // 다음 구조체의 주소
};
struct weak { // 특수공격의 해싱 테이블 원소로 사용하는 구조체
	struct soldier *soldierLoc; // 해당하는 병사 자료의 위치 
	struct weak *next; // 같은 인덱스의 다음 병사 (overflow-chaining)
};
struct binary { // 바이너리 트리 구현에 사용되는 각 노드
	int value; // 노드의 값
	struct binary *left; // 왼쪽 차일드 노드
	struct binary *right; // 오른쪽 차일드 노드
};
struct soldier squadA[5]; // A팀의 병사들의 정보들이 모임. 0번 인덱스는 사용하지 않습니다.
struct soldier squadB[5]; // B팀의 병사들의 정보들이 모임. 0번 인덱스는 사용하지 않습니다.

char map[41][41][3]; // map[0] 부분과 map[x][0] 부분들은 사용하지 않습니다.
struct soldierQ *soldierQueueHead; // 턴마다 병사들의 행동 순서가 들어가 있음. 
int round; // 현재 라운드 번호


struct weak *weakTable[10]; // 특수공격의 해싱 테이블


// 함수들의 정의
void mainGame();
void viewSquad();
void viewMain();
void soldierEnqueue(char input);
char soldierDequeue();
void damage(struct soldier *target, int damageVal);







void overload(int freq) // freq 주파수에 특수공격 실행
{
	// 주파수 테이블의 특정 주파수의 병사 모두에게 3의 데미지를 줌
	struct weak *cursor;
	cursor = weakTable[freq];
	if (cursor == NULL) // 해당 주파수의 병사가 없을 경우
	{
		printf("해당 주파수를 과부하시켰으나, 해당 주파수를 가진 대상이 없습니다.\n");
	}
	while (cursor != NULL)
	{
		if (cursor->soldierLoc->live == 1)
		{
			printf("%c의 장비가 과부하되었습니다!\n", cursor->soldierLoc->name);
			damage(cursor->soldierLoc, 3);
		}
		cursor = cursor->next;

	}
}



void traverseBinary(int *weakArray, struct binary *cursor) // 만든 주파수들의 2진 검색 트리를 cursor에서부터 검색하며, 얻은 값들을 weakArray 에 표시
{
	weakArray[cursor->value] = 1;
	if (cursor->left != NULL) // 왼쪽에 child가 존재할 경우. 재귀적 실행
	{
		traverseBinary(weakArray, cursor->left);
	}
	if (cursor->right != NULL) // 오른쪽에 child가 존재할 경우. 재귀적 실행
	{
		traverseBinary(weakArray, cursor->right);
	}
}


struct binary *putBinary(int input, struct binary *head) // input 값을 head가 시작점인 2진 검색 트리에 삽입
{
	struct binary *cursor;
	cursor = head;

	// 새 바이너리 서치 트리 노드 생성
	struct binary *current;
	current = (struct binary*)malloc(sizeof(struct binary));
	current->value = input;
	current->left = NULL;
	current->right = NULL;

	if (head == NULL) // 루트 노드를 생성할 차례일 시
	{
		head = current;
	}
	else // 새로 만든 노드를 적절한 위치에 삽입
	{
		while (1)
		{
			if (input < cursor->value) // 현재 노드에서 왼쪽으로 내려가야 할 때
			{
				if (cursor->left == NULL) // 내려갈 곳에 다른 노드가 없는 경우 -> 여기에 삽입
				{
					cursor->left = current;
					break;
				}
				else // 내려갈 곳에 다른 노드가 있는 경우 - 그곳으로 이동
				{
					cursor = cursor->left;
				}
			}

			else if (input > cursor->value) // 현재 노드에서 오른쪽으로 내려가야 할 때
			{
				if (cursor->right == NULL) // 내려갈 곳에 다른 노드가 없는 경우 -> 여기에 삽입
				{
					cursor->right = current;
					break;
				}
				else // 내려갈 곳에 다른 노드가 있는 경우 - 그곳으로 이동
				{
					cursor = cursor->right;
				}
			}
		}
	}

	return head; // 트리의 헤드 위치 리턴
}


void massiveOverload(int input, int isBigger) // 대규모 과부하 실행, isBigger 이 0인 경우 input미만의 주파수 대상, 1인 경우 input초과의 주파수 대상
{
	int i;
	int *weakArray; // 10개의 각 주파수 대역별로 해당 주파수를 가진 병사가 있는지를 0과 1로 표현
	weakArray = (int*)malloc(sizeof(int) * 10);
	struct binary *head = NULL; // 바이너리 서치 트리의 루트 노드 위치
	struct binary *cursor = NULL;




	// weakArray 0으로 모두 초기화
	for (i = 0; i <= 9; i++)
	{
		weakArray[i] = 0;
	}

	// 각 주파수 대역별로 해당 주파수를 가진 병사가 있는지를 0과 1로 표현
	for (i = 1; i <= 4; i++)
	{
		weakArray[squadA[i].weak] = 1;
		weakArray[squadB[i].weak] = 1;
	}


	head = putBinary(input, head); // 입력받았던 주파수 대역을 바이너리 서치 트리에 삽입


	// 각 실제로 사용되는 주파수 대역들을 바이너리 서치 트리에 삽입
	for (i = 0; i <= 9; i++)
	{
		if ((weakArray[i] == 1) && (i != input))
		{
			putBinary(i, head);
		}
	}



	// weakArray 0으로 다시 모두 초기화
	for (i = 0; i <= 9; i++)
	{
		weakArray[i] = 0;
	}

	if (isBigger == 0) // 입력값보다 작은 주파수들을 찾는 경우
	{
		if (head->left != NULL)
		{
			cursor = head->left;
			traverseBinary(weakArray, cursor);
		}
	}
	else if (isBigger == 1) // 입력값보다 큰 주파수들을 찾는 경우
	{
		if (head->right != NULL)
		{
			cursor = head->right;
			traverseBinary(weakArray, cursor);
		}
	}
	else
	{
		printf("isBigger 값이 잘못되었습니다.");
		system("pause");

	}

	// 가려낸 모든 주파수 대역대에 특수공격 실행
	for (i = 0; i <= 9; i++)
	{
		if (weakArray[i] == 1)
		{
			overload(i);
		}
	}

}



void heapInsert(int input, int *workBench, int seq) // min-heap tree 를 input에서 데이터들을 끌어와 생성하고 유지
{
	// input 이번에 트리에 추가할 데이터
	// workBench input값들로 heap tree 구현하는 장소
	// seq 이번 삽입 작업의 순서 번호
	int current; // 현재 작업하는 포인터
	int temp;
	seq++;

	//입력받은 input 값을 트리에 삽입
	workBench[seq] = input;
	current = seq;

	while (1) // min heap 의 규칙에 위배되지 않을 때까지 계속 위와 아래 원소를 교환 (아래부터)
	{
		if (current == 1) // 삽입했던 원소가 맨 위까지 올라온 경우
		{
			break;
		}

		if (workBench[current] >= workBench[current / 2]) // min heap의 규칙이 충족될 경우
		{
			break;
		}
		else // 위와 아래를 교환해야 하는 경우
		{
			temp = workBench[current];
			workBench[current] = workBench[current / 2];
			workBench[current / 2] = temp;
			current = current / 2;
		}

	}
}



void heapDelete(int *workBench, int *output, int seq, int dataSize) // min-heap tree 에서 데이터들을 삭제하며 output에 계속 삽입. output에는 오름차순으로 데이터가 적립됨
{

	// workBench input값의 heap tree
	// seq 이번 삭제 작업의 순서 번호
	int heapSeq = seq + 1;

	// 힙 트리 정상의 값을 하나 삭제해 output에 삽입하고, 그 자리에 맨 마지막으로 들어온 값을 삽입
	output[dataSize - seq - 1] = workBench[1];
	workBench[1] = workBench[heapSeq];
	int current = 1;
	int temp;

	while (1) // min heap 의 규칙에 위배되지 않을 때까지 계속 위와 아래 원소를 교환 (위부터)
	{
		if ((current * 2) > heapSeq) // 맨 위에 있던 원소가 끝까지 내려옴
		{
			break;
		}

		if ((workBench[current] <= workBench[current * 2])) // min heap의 규칙이 맞는 경우
		{
			if ((current * 2 + 1) == (heapSeq + 1)) // 현재 위치의 오른쪽 자식이 없는 경우
			{
				break;
			}
			else // 현재 위치에 오른쪽 자식도 있는 경우
			{
				if (workBench[current] <= workBench[current * 2 + 1])
				{
					break;
				}
			}
		}

		// 교환해야 하는 경우
		{
			if (workBench[current * 2] <= workBench[current * 2 + 1]) // 왼쪽 자식이 오른쪽 자식보다 더 작거나 같은 경우
			{
				temp = workBench[current];
				workBench[current] = workBench[current * 2];
				workBench[current * 2] = temp;
				current *= 2;
			}

			else // 오른쪽 자식이 왼쪽 자식보다 더 작은 경우
			{
				if ((current * 2 + 1) != (heapSeq + 1)) // 현재 위치의 오른쪽 자식이 있는 경우
				{
					temp = workBench[current];
					workBench[current] = workBench[current * 2 + 1];
					workBench[current * 2 + 1] = temp;
					current = (current * 2 + 1);
				}
			}
		}


	}
}



int *heapSort(int *input, int dataSize) // heap sort 실행
{



	int *output;
	int *workBench; // workBench 배열은 인덱스를 1부터 사용함(혼동 방지)
	int i;


	workBench = (int*)malloc(sizeof(int) * (dataSize + 2));
	output = (int*)malloc(sizeof(int) * dataSize);
	if ((output == NULL) || (workBench == NULL))
	{
		printf("Memory allocation failed. Please restart this program\n");
		getchar();
	}


	for (i = 0; i < dataSize; i++) // input의 각 값들을 순서대로 힙 트리에 추가
	{
		heapInsert(input[i], workBench, i);
	}



	for (i = (dataSize - 1); i >= 0; i--)
	{
		heapDelete(workBench, output, i, dataSize); // 힙 트리의 각 값들을 output에 추가. output에는 값들이 오름차순으로 정렬되게 됨.
	}




	free(input);

	free(workBench);

	return output;

}



void resetTime() // 게임 체인저 - 시간 역행 실행 - 현재 쓰고 있던 병사 행동 큐를 다시 정렬해 넣음
{
	int i, j;

	int *dexQueue; // 정렬할 민첩 능력치들이 삽입됨
	dexQueue = (int*)malloc(sizeof(int) * 8);
	// 정렬할 각 민첩 능력치 자료 삽입
	for (i = 0; i <= 3; i++)
	{
		dexQueue[i] = squadA[i + 1].DEX;
		dexQueue[i + 4] = squadB[i + 1].DEX;
	}

	dexQueue = heapSort(dexQueue, 8); // 자료들을 heap sort

	while (soldierQueueHead != NULL) // 병사 행동 큐의 원소 모두 제거
	{
		soldierDequeue();
	}


	


	// 각 민첩 능력치에 해당하는 병사들을 정렬된 대로 행동 큐에 Enqueue
	for (i = 0; i <= 7; i++)
	{
		j = 0;
		while (1)
		{
			if ((squadA[j + 1].DEX == dexQueue[7 - i]) || (squadB[j + 1].DEX == dexQueue[7 - i]))
			{
				if (squadA[j + 1].DEX == dexQueue[7 - i])
				{
					
					soldierEnqueue(squadA[j + 1].name);
					break;
				}
				else
				{
					
					soldierEnqueue(squadB[j + 1].name);
					break;
				}

			}

			j++;
		}

		
	}

	free(dexQueue);
}


void stackPush(struct soldier *current) // 병사의 탄창에 탄환 추가 작업
{
	current->stackTop++;
	if (current->stackTop > 4)
	{
		printf("에러 - 탄환 수가 초과되었습니다.\n");
		system("pause");
	}
}


void stackPop(struct soldier *current) // 병사의 사격 후 탄창에서 탄환을 빼는 작업
{
	current->stackTop--;
	if (current->stackTop <= -2)
	{
		printf("에러 - 남은 탄환 수가 음수입니다.\n");
		system("pause");
	}
}


void weakAdd(struct soldier *input) // 주파수 해싱 테이블에 input으로 들어온 병사를 적절한 인덱스에 삽입
{
	int index = input->weak;

	// 새로 들어온 병사의 키값 셀 생성
	struct weak *currentCell;
	currentCell = (struct weak*)malloc(sizeof(struct weak));
	currentCell->soldierLoc = input;
	currentCell->next = NULL;

	if (weakTable[index] == NULL) // 해당 인덱스의 맨 처음 삽입일 경우
	{
		weakTable[index] = currentCell;
	}
	else // 해당 인덱스에 다른 원소가 이미 있을 경우 - overflow chaining
	{
		struct weak *cursor;
		cursor = weakTable[index];
		while (cursor->next != NULL)
		{
			cursor = cursor->next;
		}
		cursor->next = currentCell;
	}
}



void soldierEnqueue(char input) // 입력받은 이름의 병사를 행동 순서 큐 맨 뒤에 삽입
{
	if (soldierQueueHead == NULL) // 큐에 원소가 없는 경우
	{
		struct soldierQ *element;
		element = (struct soldierQ*)malloc(sizeof(struct soldierQ));
		element->name = input;
		element->next = NULL;
		soldierQueueHead = element;

	}
	else // 큐에 다른 원소가 있는 경우 - 큐 맨 뒤에 삽입
	{
		struct soldierQ *element;
		element = (struct soldierQ*)malloc(sizeof(struct soldierQ));
		element->name = input;
		element->next = NULL;


		struct soldierQ *cursor = soldierQueueHead;
		while (cursor->next != NULL)
		{
			cursor = cursor->next;
		}
		cursor->next = element;
	}
}


char soldierDequeue() // 행동 큐 맨 앞의 병사를 큐에서 삭제 (주로 다음 턴으로 넘길 때 사용)
{
	int output;
	struct soldierQ *temp;
	if (soldierQueueHead->next == NULL) // 병사 행동 큐에 원소가 하나뿐인 경우의 삭제
	{
		output = soldierQueueHead->name;

		free(soldierQueueHead);
		soldierQueueHead = NULL;
	}
	else // 병사 행동 큐에 다른 원소가 있는 경우의 삭제
	{
		output = soldierQueueHead->name;
		temp = soldierQueueHead->next;
		free(soldierQueueHead);
		soldierQueueHead = temp;
	}


	return output;
}



void merge(int *input, int *workBench, int start, int end) // 각 칸들을 재귀적으로 merge하는 함수, 작은 그룹부터 큰 그룹의 순서로 각각 정렬한다
{
	// input은 정렬할 랜덤 자료들
	// workBench는 임시 작업 장소. 정렬을 workBench 에서 진행해 끝나면 input으로 정렬된 자료를 옮긴다.
	// start 현재 pass의 merge 함수를 실행할 input 배열 내의 시작 지점
	// end 현재 pass의 merge 함수를 실행할 input 배열 내의 끝 지점
	int i, j, k;
	int temp;

	if (((end - start) != 1) && ((end - start) != 2)) // 현재 패스의 자료 갯수가 0개나 1개가 아니라면, 재귀적으로 좌측 그룹과 우측 그룹의 merge 실행
	{
		merge(input, workBench, start, ((start + end) / 2)); // 좌측 그룹의 merge 실행
		merge(input, workBench, ((start + end) / 2) + 1, end); // 우측 그룹의 merge 실행
	}

	if ((end - start) == 2) // 현재 계산중인 그룹의 원소가 3개일 경우, 후속 정렬을 위해 미리 첫째와 둘째 원소를 정렬해 놓는다.
	{
		if (input[start] > input[start + 1])
		{
			temp = input[start];
			input[start] = input[start + 1];
			input[start + 1] = temp;
		}
	}

	// 앞의 작은 그룹들의 재귀적 merge 함수 처리가 완료되면(작은 그룹들이 정렬이 완료되면), 현재 그룹의 정렬 실행
	// 앞 그룹과 뒤 그룹의 맨 앞 값이 누가 더 작은지 비교하고, 둘 중 작은 것을 workBench 에 넣는 것을 반복한다.
	i = start; // 앞쪽 그룹의 커서
	j = ((start + end) / 2) + 1; // 뒤쪽 그룹의 커서
	k = start;
	while (1)
	{


		if (input[i] < input[j]) // 앞쪽 그룹의 맨 앞 값이 더 작음
		{
			workBench[k] = input[i];
			i++;
		}
		else // 뒤쪽 그룹의 맨 앞 값이 더 작음
		{
			workBench[k] = input[j];
			j++;
		}
		k++;


		if (i == (((start + end) / 2) + 1)) // 앞쪽 그룹의 값들이 모두 소진된 경우
		{
			while (k != (end + 1))
			{
				workBench[k] = input[j];
				j++;
				k++;
			}
			break;
		}
		else if (j == end + 1) // 뒤쪽 그룹의 값들이 모두 소진된 경우
		{
			while (k != (end + 1))
			{
				workBench[k] = input[i];
				i++;
				k++;
			}
			break;
		}
	}


	for (i = start; i <= end; i++) // workBench에서 작업한 내용들을 input으로 옮김
	{
		input[i] = workBench[i];
	}
}



int *mergeSort(int *input, int dataSize) // merge sort 실행
{

	int *workBench; // 정렬 작업에 사용하는 임시 공간


	workBench = (int*)malloc(sizeof(int) * dataSize);
	if (workBench == NULL)
	{
		printf("Memory allocation failed. Please restart this program\n");
		system("pause");
	}


	merge(input, workBench, 0, (dataSize - 1)); // 재귀적으로 가장 작은 그룹에서 큰 그룹 순서로 그룹들을 정렬하는 함수


	free(workBench);

	return input;

}



void rocket(int targetSide) // 게임 체인저 - 로켓 런처 실행. Trie 구조로 targetSide 사분면의 모든 엄폐물을 파괴
{
	char **trieHead[5]; // Trie의 헤드. 0번 인덱스는 사용하지 않습니다.
	int i, j;

	// 각 사분면 별 해당하는 map 데이터를 담을 공간 할당
	trieHead[1] = (char**)malloc(sizeof(char*) * 400);
	trieHead[2] = (char**)malloc(sizeof(char*) * 400);
	trieHead[3] = (char**)malloc(sizeof(char*) * 400);
	trieHead[4] = (char**)malloc(sizeof(char*) * 400);
	// 배열을 채우는 데 사용하는 카운터
	int counter1 = 0;
	int counter2 = 0;
	int counter3 = 0;
	int counter4 = 0;

	// map의 주소들을 각각 맞는 사분면에 삽입
	for (i = 1; i <= 40; i++)
	{
		for (j = 1; j <= 40; j++)
		{
			if (i <= 20 && j <= 20) // 해당 좌표가 1사분면인 경우
			{
				trieHead[1][counter1] = map[i][j];
				counter1++;
			}
			else if (i <= 20 && j > 20) // 해당 좌표가 2사분면인 경우
			{
				trieHead[2][counter2] = map[i][j];
				counter2++;
			}
			else if (i > 20 && j > 20) // 해당 좌표가 3사분면인 경우
			{
				trieHead[3][counter3] = map[i][j];
				counter3++;
			}
			else if (i > 20 && j <= 20) // 해당 좌표가 4사분면인 경우
			{
				trieHead[4][counter4] = map[i][j];
				counter4++;
			}
		}
	}

	// 엄폐물 파괴 작업
	for (i = 0; i <= 399; i++)
	{
		if ((strcmp(trieHead[targetSide][i], "■") == 0) || (strcmp(trieHead[targetSide][i], "▦") == 0)) // 타켓 사분면의 해당 위치가 엄폐물일 경우, 파괴
		{
			strcpy_s(trieHead[targetSide][i], sizeof("□"), "□");
		}
	}

}



void viewMap() // 맵을 표시하는 함수
{
	int i, j;

	for (i = 1; i <= 40; i++)
	{
		for (j = 1; j <= 40; j++)
		{
			printf("%s", map[i][j]);
		}
	}
	printf("\n");
}




void nextTurn() // 다음 턴으로 진행하는 데 사용하는 함수
{
	

	// 병사 큐의 맨 앞의 병사를 deQueue하고 그 큐의 맨 뒤에 다시 enQueue 실행
	soldierEnqueue(soldierDequeue());


	mainGame(); // 게임 화면으로 돌아감 -> 다음 턴
}



void damage(struct soldier *target, int damageVal) // 피해를 주는 데 사용하는 함수. target의 체력을 damageVal 만큼 감소시키고 사망 판정
{
	target->curHP -= damageVal;

	printf("%c가 %d의 데미지를 입었습니다.\n", target->name, damageVal);

	if (target->curHP < 0) // 타겟 사망 시
	{
		target->live = 0;
		printf("%c의 HP가 소진되어 쓰러졌습니다!\n", target->name);

		//지도위치 지우기
		strcpy_s(map[target->location[0]][target->location[1]], sizeof("□"), "□");
	}



}



void mainGame() // 진행 중인 게임의 메인 화면
{

	system("cls");


	// 승패 판정
	if (squadA[1].live == 0 && squadA[2].live == 0 && squadA[3].live == 0 && squadA[4].live == 0)
	{
		printf("축하합니다! B 팀이 승리했습니다!\n");
		system("pause");




		viewMain();
	}
	if (squadB[1].live == 0 && squadB[2].live == 0 && squadB[3].live == 0 && squadB[4].live == 0)
	{
		printf("축하합니다! A 팀이 승리했습니다!\n");
		system("pause");




		viewMain();
	}


	

	int choice; // 이번 턴의 행동 선택에 사용하는 변수
	int i, j;
	struct soldier *current; // 이번 턴에 행동하고 있는 병사
	
	// 이번 턴에 행동하는 병사 데이터를 current 에 불러옴
	switch (soldierQueueHead->name)
	{
	case '1':
		current = &squadA[1];
		break;
	case '2':
		current = &squadA[2];
		break;
	case '3':
		current = &squadA[3];
		break;
	case '4':
		current = &squadA[4];
		break;
	case 'A':
		current = &squadB[1];
		break;
	case 'B':
		current = &squadB[2];
		break;
	case 'C':
		current = &squadB[3];
		break;
	case 'D':
		current = &squadB[4];
		break;
	default:
		current = NULL;
		break;
	}

	// 불려온 병사가 이미 사망한 병사인 경우, 다음 병사의 턴으로 바로 진행
	if (current->live == 0)
	{
		nextTurn();
	}

	// 게임 메인 화면 출력
	viewMap(); // 맵 출력
	printf("\n\n");
	printf("%d 라운드\n", round);
	printf("%c의 턴입니다.\n", current->name);
	printf("1. 이동 2. 사격 3. 특수공격 4. 병사 정보 확인 5. 재장전 6. 게임체인저\n -> ");
	scanf_s("%d", &choice);
	while (choice != 1 && choice != 2 && choice != 3 && choice != 4 && choice != 5 && choice != 6) // 잘못된 옵션 선택 경우
	{
		printf("잘못된 옵션입니다. 올바른 행동을 선택해 주세요\n");
		printf("1. 이동 2. 사격 3. 특수공격 4. 병사 정보 확인 5. 재장전 6. 게임체인저\n -> ");
		scanf_s("%d", &choice);
	}

	if (choice == 1) // 이동
	{
		int moveX; // 이동할 x축 값
		int moveY; // 이동할 y축 값
		int afterX;
		int afterY;

		while (1)
		{
			printf("이동할 x축 값을 입력해 주세요 (-5~5)\n");
			scanf_s("%d", &moveX);
			while (moveX < -5 || moveX > 5)
			{
				printf("올바른 값을 입력해 주세요.\n");
				scanf_s("%d", &moveX);
			}
			printf("이동할 y축 값을 입력해 주세요 (-5~5)\n");
			scanf_s("%d", &moveY);
			while (moveY < -5 || moveY > 5)
			{
				printf("올바른 값을 입력해 주세요.\n");
				scanf_s("%d", &moveY);
			}




			afterX = current->location[0] - moveY;
			afterY = current->location[1] + moveX;

			// 이동하려고 하는 곳이 빈 칸이 아닌 경우
			if (afterX > 40 || afterY > 40 || afterX < 0 || afterX < 0 || strcmp(map[afterX][afterY], "□") != 0)
			{
				printf("그곳으로는 이동할 수 없습니다.\n");
				continue;
			}
			// 해당 칸으로 이동 가능한 경우, 이동하고 위치 변경 작업 실행
			else
			{
				// 맵 갱신
				strcpy_s(map[current->location[0]][current->location[1]], sizeof("□"), "□");

				// 병사의 위치 정보 갱신
				current->location[1] += moveX;
				current->location[0] -= moveY;

				// 맵 갱신
				if (current->name == '1')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "１");
					break;
				}
				else if (current->name == '2')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "２");
					break;
				}
				else if (current->name == '3')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "３");
					break;
				}
				else if (current->name == '4')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "４");
					break;
				}
				else if (current->name == 'A')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "Ａ");
					break;
				}
				else if (current->name == 'B')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "Ｂ");
					break;
				}
				else if (current->name == 'C')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "Ｃ");
					break;
				}
				else if (current->name == 'D')
				{
					strcpy_s(map[afterX][afterY], sizeof("□"), "Ｄ");
					break;
				}



				break;
			}
		}
	}
	else if (choice == 2) // 사격
	{
		char targetN; // 사격 대상의 이름
		struct soldier *target = NULL; // 사격 대상의 정보

		// 장탄수 확인
		if (current->stackTop == -1) // 탄창이 빈 경우 - 사격 불가
		{
			printf("탄창이 비었습니다. 재장전을 해야 사격이 가능합니다.\n");
			mainGame();
		}

		while (1)
		{
			printf("사격 대상을 입력하세요(숫자 또는 대문자)\n");
			printf("1 2 3 4 A B C D\n -> ");
			scanf_s("%c", &targetN, sizeof(char));
			scanf_s("%c", &targetN, sizeof(char));
			if (targetN != '1' && targetN != '2' && targetN != '3' && targetN != '4' && targetN != 'A' && targetN != 'B' && targetN != 'C' && targetN != 'D')
			{
				printf("잘못된 입력입니다.\n");
				continue;
			}

			// 입력받은 타켓 이름으로 타겟의 정보를 불러옴
			for (i = 1; i <= 4; i++)
			{
				if (squadA[i].name == targetN)
				{
					target = &squadA[i];
					break;
				}
				else if (squadB[i].name == targetN)
				{
					target = &squadB[i];
					break;
				}

			}



			if (current->name == targetN) // 자신을 쏘려 하는 경우
			{
				printf("당신은 소중한 생명입니다.\n");
			}
			else if (current->team == target->team) // 아군을 쏘려 하는 경우
			{
				printf("아군을 쏘는 것은 옳지 않습니다.\n");
			}
			else if (target->live == 0) // 이미 사망한 인원을 사격하려는 경우
			{
				printf("확인사살은 제네바 조약에 어긋납니다.\n");
			}
			else // 사격 대상이 valid 한 경우
			{
				break;
			}
		}



		int covered; // 사격 대상이 엄폐한 정도

		// 엄폐 판정 작업
		if (current->location[1] > target->location[1] && current->location[0] > target->location[0]) // 적이 제 1사분면에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0]][target->location[1] + 1], "■") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0]][target->location[1] + 1], "▦") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] < target->location[1] && current->location[0] > target->location[0]) // 적이 제 2사분면에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0]][target->location[1] - 1], "■") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0]][target->location[1] - 1], "▦") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] < target->location[1] && current->location[0] < target->location[0]) // 적이 제 3사분면에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0]][target->location[1] - 1], "■") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0]][target->location[1] - 1], "▦") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] > target->location[1] && current->location[0] < target->location[0]) // 적이 제 4사분면에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0]][target->location[1] + 1], "■") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0]][target->location[1] + 1], "▦") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] < target->location[1] && current->location[0] == target->location[0]) // 적이 바로 동쪽에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0]][target->location[1] - 1], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0]][target->location[1] - 1], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] > target->location[1] && current->location[0] == target->location[0]) // 적이 바로 서쪽에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0]][target->location[1] + 1], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0]][target->location[1] + 1], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] == target->location[1] && current->location[0] < target->location[0]) // 적이 바로 남쪽에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0] - 1][target->location[1]], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0] - 1][target->location[1]], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] == target->location[1] && current->location[0] > target->location[0]) // 적이 바로 북쪽에 위치
		{
			// 완전 엄폐 경우
			if (strcmp(map[target->location[0] + 1][target->location[1]], "■") == 0)
			{
				covered = 2;
			}
			// 반엄폐 경우
			else if (strcmp(map[target->location[0] + 1][target->location[1]], "▦") == 0)
			{
				covered = 1;
			}
			// 무엄폐 경우
			else
			{
				covered = 0;
			}
		}
		else
		{
			printf("error on location\n");
			system("pause");
		}

		// 명중률 계산
		int aim = current->aim - (covered * 20);


		printf("발사합니다...\n");
		printf("병사의 조준 +%d%%\n", current->aim);
		if (covered == 1) // 적 반엄폐 경우
		{
			printf("적 반엄폐 -20%%\n");
		}
		else if (covered == 2) // 적 완전 엄폐 경우
		{
			printf("적 완전 엄폐 -40%%\n");
		}
		printf("-명중률 %d%%\n", aim);

		int crit = 10;
		if (covered == 0) // 적 무엄폐 경우
		{
			crit += 40;
		}

		printf("기본 치명타율 +10%%\n");
		if (covered == 0) // 적 무엄폐 경우
		{
			printf("적 무엄폐 +40%%\n");
		}
		printf("-치명타율 %d%%\n", crit);
		printf("엔터를 눌러 발사합니다.\n");
		system("pause");



		// 사격의 명중 여부와 치명타 여부 판정
		int luck = rand() % 101;
		int isCrit; // 치명타 여부를 0과 1로 나타냄
		if (crit >= luck) // 치명타 판정
		{
			isCrit = 1;
		}
		else // 치명타가 아닌 경우
		{
			isCrit = 0;
		}

		int damageVal = current->ATK; // 입힐 데미지
		luck = rand() % 101;


		if (aim >= luck)
		{
			//맞음
			if (isCrit == 1) // 치명타
			{
				printf("치명타!\n");
				damageVal *= 2;
			}
			else // 단순 명중
			{
				printf("명중!\n");
			}

			damage(target, damageVal);


		}
		else
		{
			//빗맞음
			printf("빗나감!\n");
		}

		// 장탄수 소비 계산
		current->stackTop--;

		
		system("pause");

	}
	else if (choice == 3) // 특수공격
	{
		int freq; // 특수공격을 실행할 주파수

		printf("해당하는 주파수 대역에 과부하 공격을 개시합니다...\n");
		printf("과부하시킬 주파수 대역을 입력하세요 (0~9)\n");
		scanf_s("%d", &freq);
		while (0 > freq || freq > 9)
		{
			printf("잘못된 주파수 대역입니다.\n");
			printf("과부하시킬 주파수 대역을 입력하세요 (0~9)\n");
			scanf_s("%d", &freq);
		}

		overload(freq); // 특수공격 실행
		
		system("pause");
	}
	else if (choice == 4) // 대원 정보 확인
	{
		viewSquad();
	}

	else if (choice == 5) // 재장전 - 탄약이 꽉 찰 때까지 탄약 추가
	{
		printf("총기의 탄약을 채웠습니다!\n");
		while (current->stackTop != 4)
		{
			stackPush(current);
		}
		system("pause");
	}

	else if (choice == 6) // 게임 체인저(랜덤 스킬) 발동
	{
		int randChoice;
		randChoice = rand() % 3;

		if (randChoice == 0) // 게임 체인저 - 시간 역행 발동
		{
			printf("시간 역행 발동 - 턴이 맨 처음 순서부터 다시 시작됩니다.\n");
			resetTime(); // 시간 역행 진행
			system("pause");
			mainGame(); 
		}
		else if (randChoice == 1) // 게임 체인저 - 광역 과부하 발동
		{
			int freq; // 대상 주파수
			int massOption; // 대상 주파수보다 큰/작은 범위 공격 여부

			printf("광역 과부하 발동 - 특정 주파수보다 작거나 큰 모든 주파수를 과부하시킵니다.\n");
			printf("과부하시킬 기준 주파수 대역을 입력하세요 (0~9)\n");
			scanf_s("%d", &freq);
			while (0 > freq || freq > 9)
			{
				printf("잘못된 주파수 대역입니다.\n");
				printf("과부하시킬 주파수 대역을 입력하세요 (0~9)\n");
				scanf_s("%d", &freq);
			}

			printf("기준 주파수보다 큰 주파수들을 교란하려면 1을, 작은 주파수들을 교란하려면 0을 입력하세요\n");
			scanf_s("%d", &massOption);
			while (massOption != 0 && massOption != 1)
			{
				printf("잘못된 주파수 대역입니다.\n");
				printf("기준 주파수보다 큰 주파수들을 교란하려면 1을, 작은 주파수들을 교란하려면 0을 입력하세요\n");
				scanf_s("%d", &massOption);

			}

			massiveOverload(freq, massOption); // 해당 파라미터로 광역 과부하 실행
			system("pause");
		}
		else if (randChoice == 2) // 게임 체인저 - 로켓 런처 발사
		{
			int targetSide; // 대상 사분면

			printf("로켓 런처 사용 - 특정 사분면의 모든 엄폐물들을 파괴합니다.\n");
			printf("엄폐물들을 파괴할 사분면을 입력해 주세요.\n");
			scanf_s("%d", &targetSide);
			while (targetSide != 1 && targetSide != 2 && targetSide != 3 && targetSide != 4)
			{
				printf("잘못된 입력값입니다.\n");
				printf("엄폐물들을 파괴할 사분면을 입력해 주세요.\n");
				scanf_s("%d", &targetSide);
			}

			rocket(targetSide); // 로켓 런처 발사 실행
			printf("해당 사분면의 엄폐물들이 폭발로 모두 파괴되었습니다!\n");
			system("pause");
		}
		
	}

	nextTurn(); // 이번 병사의 행동이 끝남. 다음 병사의 턴으로 넘어감
}



void viewSquad() // 병사들 정보 열람
{
	int i;


	printf("\n병사 정보\n\n");
	printf("이름 최대HP 현재HP 조준 화력 민첩 주파수 장탄수\n");
	for (i = 1; i <= 4; i++)
	{
		printf("%c    %d     %d     %d   %d    %d   %d      %d\n", squadA[i].name, squadA[i].maxHP, squadA[i].curHP, squadA[i].aim, squadA[i].ATK, squadA[i].DEX, squadA[i].weak, (squadA[i].stackTop) + 1);
	}
	for (i = 1; i <= 4; i++)
	{
		printf("%c    %d     %d     %d   %d    %d   %d      %d\n", squadB[i].name, squadB[i].maxHP, squadB[i].curHP, squadB[i].aim, squadB[i].ATK, squadB[i].DEX, squadB[i].weak, (squadB[i].stackTop) + 1);
	}

	system("pause");
	mainGame();
}




void initSoldier(struct soldier *squad) // 입력받은 병사의 능력치를 무작위로 초기화
{
	/*
	HP의 범위 -> 15~20
	조준의 범위 -> 50~80
	화력의 범위 -> 4~6
	민첩의 범위 -> 0~99
	약점의 범위 -> 0~9
	*/
	
	squad->maxHP = rand() % 6;
	squad->maxHP += 15;
	squad->curHP = squad->maxHP;
	squad->aim = rand() % 31;
	squad->aim += 50;
	squad->ATK = rand() % 3;
	squad->ATK += 4;
	squad->DEX = rand() % 100;
	squad->weak = rand() % 10;
	squad->live = 1;
	squad->stackTop = 4;
}



void initGame() // 게임 시작시에 각 변수 초기화, 맵 불러오기, 병사 배치 작업
{
	// 전 판 변수 초기화도 진행
	while (soldierQueueHead != NULL) // 병사 행동 큐의 원소 모두 제거
	{
		soldierDequeue();
	}

	system("cls");



	int i, j;
	char temp;

	// 주파수 해싱 테이블 초기화
	for (i = 0; i <= 9; i++)
	{
		weakTable[i] = NULL;
	}


	round = 1;

	srand((unsigned)time(NULL));


	// 맵 파일 읽어옴
	FILE *fp;
	fopen_s(&fp, "map.txt", "r");
	for (i = 1; i <= 40; i++)
	{
		
		for (j = 1; j <= 40; j++)
		{
			fscanf_s(fp, "%c", &map[i][j][0], sizeof(char));
			fscanf_s(fp, "%c", &map[i][j][1], sizeof(char));
			map[i][j][2] = '\0';
			
		}
		fscanf_s(fp, "%c", &temp, sizeof(char));
		

	}
	


	fclose(fp);

	// 병사들의 능력치 초기화
	int isOverLap;
	while (1)
	{
		isOverLap = 0;
		for (i = 1; i <= 4; i++)
		{
			initSoldier(&squadA[i]);
			initSoldier(&squadB[i]);
		}

		// 민첩 능력치의 중복 여부 검사 - 중복시 능력치 재설정
		for (i = 1; i <= 4; i++)
		{
			for (j = 1; j <= 4; j++)
			{
				if (((squadA[i].DEX == squadA[j].DEX) || (squadA[i].DEX == squadB[j].DEX)) && (i != j))
				{
					isOverLap = 1;
				}

				if (((squadB[i].DEX == squadA[j].DEX) || (squadB[i].DEX == squadB[j].DEX)) && (i != j))
				{
					isOverLap = 1;
				}
			}

		}


		if (isOverLap == 0)
		{
			break;
		}
	}

	// 주파수 해싱 테이블에 병사들의 주파수 삽입
	for (i = 1; i <= 4; i++)
	{
		weakAdd(&squadA[i]);
		weakAdd(&squadB[i]);
	}

	// 병사들의 이름/팀 초기화
	squadA[1].name = '1';
	squadA[2].name = '2';
	squadA[3].name = '3';
	squadA[4].name = '4';
	squadB[1].name = 'A';
	squadB[2].name = 'B';
	squadB[3].name = 'C';
	squadB[4].name = 'D';
	for (i = 1; i <= 4; i++)
	{
		squadA[i].team = 'A';
		squadB[i].team = 'B';
	}





	// 민첩 능력치의 merge sort를 이용한 정렬
	int *dexQueue;
	dexQueue = (int*)malloc(sizeof(int) * 8);
	// 정렬할 자료 삽입
	for (i = 0; i <= 3; i++)
	{
		dexQueue[i] = squadA[i + 1].DEX;
		dexQueue[i + 4] = squadB[i + 1].DEX;
	}


	dexQueue = mergeSort(dexQueue, 8);

	// 정렬된 민첩 능력치에 해당하는 병사들을 행동 큐에 순서대로 Enqueue 함
	for (i = 0; i <= 7; i++)
	{
		j = 0;
		while (1)
		{
			if ((squadA[j + 1].DEX == dexQueue[7 - i]) || (squadB[j + 1].DEX == dexQueue[7 - i]))
			{
				if (squadA[j + 1].DEX == dexQueue[7 - i])
				{
					
					soldierEnqueue(squadA[j + 1].name);
					break;
				}
				else
				{
					
					soldierEnqueue(squadB[j + 1].name);
					break;
				}

			}


			j++;
		}

		
	}



	free(dexQueue);


	// 병사들을 맵에 랜덤하게 배치
	for (i = 1; i <= 4; i++)
	{
		while (1)
		{
			squadA[i].location[0] = rand() % 40 + 1;
			squadA[i].location[1] = rand() % 40 + 1;
			if (strcmp(map[squadA[i].location[0]][squadA[i].location[1]], "□") == 0)
			{
				if (i == 1)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("□"), "１");
					break;
				}
				else if (i == 2)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("□"), "２");
					break;
				}
				else if (i == 3)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("□"), "３");
					break;
				}
				else if (i == 4)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("□"), "４");
					break;
				}

			}
		}

	}




	// 병사들을 맵에 랜덤하게 배치
	for (i = 1; i <= 4; i++)
	{
		while (1)
		{
			squadB[i].location[0] = rand() % 40 + 1;
			squadB[i].location[1] = rand() % 40 + 1;
			if (strcmp(map[squadB[i].location[0]][squadB[i].location[1]], "□") == 0)
			{
				if (i == 1)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("□"), "Ａ");
					break;
				}
				else if (i == 2)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("□"), "Ｂ");
					break;
				}
				else if (i == 3)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("□"), "Ｃ");
					break;
				}
				else if (i == 4)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("□"), "Ｄ");
					break;
				}

			}
		}

	}



	viewSquad(); // 게임 시작 시 병사들의 정보를 보여주는 창으로 진행
}



void viewRule() // 룰북 보기 - rule.txt를 불러옴 - 그 후 다시 메인 메뉴로
{
	system("cls");
	char buffer[100];

	FILE *fp;
	fopen_s(&fp, "rule.txt", "r");
	while (!feof(fp))
	{
		fgets(buffer, sizeof(buffer), fp);
		printf("%s", buffer);
	}

	system("pause");
	viewMain();
}




void viewMain() // 메인 화면을 표시하고, 해당하는 메뉴로 이동합니다.
{
	system("cls");

	int choice; // 선택값을 입력받음

	printf("  ■      ■       ■■■■■  ■■■■■  ■       ■\n");
	printf("   ■    ■        ■          ■      ■  ■■   ■■\n");
	printf("    ■  ■         ■          ■      ■  ■ ■ ■ ■\n");
	printf("     ■■          ■          ■      ■  ■  ■   ■\n");
	printf("      ■   ■■■  ■          ■      ■  ■  ■   ■\n");
	printf("     ■■          ■          ■      ■  ■  ■   ■\n");
	printf("    ■  ■         ■          ■      ■  ■  ■   ■\n");
	printf("   ■    ■        ■          ■      ■  ■  ■   ■\n");
	printf("  ■      ■       ■■■■■  ■■■■■  ■  ■   ■\n\n\n");
	printf("X-COM CLI Edition - Developed by Gazua team\n\n\n");

	printf("메뉴를 선택하세요\n\n");
	printf("1. 게임 시작 2. 게임 규칙 보기 3. 나가기\n");
	printf("-> ");
	scanf_s("%d", &choice);
	while (choice != 1 && choice != 2 && choice != 3) // 잘못된 옵션 선택 경우
	{
		printf("잘못된 옵션입니다. 올바른 메뉴를 선택해 주세요\n");
		printf("1. 게임 시작 2. 게임 규칙 보기 3. 나가기\n");
		printf("-> ");
		scanf_s("%d", &choice);
	}

	if (choice == 1) // 게임 시작
	{
		initGame();
	}
	else if (choice == 2) // 룰북 보기
	{
		viewRule();
	}
	else if (choice == 3) // 게임 종료
	{
		exit(0);
	}
}



void main()
{
	viewMain(); // 게임 메인 화면으로 이동

}
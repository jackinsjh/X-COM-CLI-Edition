#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>


struct soldier { // �� ���縦 ��Ÿ���� ����ü
	char name; // ������ �̸�
	int maxHP; // ������ �ִ� HP
	int curHP; // ������ ���� HP
	int aim; // ������ ���� �ɷ�ġ
	int ATK; // ������ ȭ�� �ɷ�ġ
	int DEX; // ������ ��ø �ɷ�ġ
	int weak; // ������ ���ļ� �뿪
	int location[2]; // ������ ��ġ - x��� y��
	int live; // ���簡 ����ִ����� ���θ� 0�� 1�� ǥ��
	char team; // ������ �Ҽ� ���� ��Ÿ��
	int magStack[5]; // ������ ���� ��ź ��
	int stackTop; // ������ źâ ���� ������ ���
};
struct soldierQ { // ������ ���� ������ ����ϴ� ����ü
	char name; // ������ �̸�
	struct soldierQ *next; // ���� ����ü�� �ּ�
};
struct weak { // Ư�������� �ؽ� ���̺� ���ҷ� ����ϴ� ����ü
	struct soldier *soldierLoc; // �ش��ϴ� ���� �ڷ��� ��ġ 
	struct weak *next; // ���� �ε����� ���� ���� (overflow-chaining)
};
struct binary { // ���̳ʸ� Ʈ�� ������ ���Ǵ� �� ���
	int value; // ����� ��
	struct binary *left; // ���� ���ϵ� ���
	struct binary *right; // ������ ���ϵ� ���
};
struct soldier squadA[5]; // A���� ������� �������� ����. 0�� �ε����� ������� �ʽ��ϴ�.
struct soldier squadB[5]; // B���� ������� �������� ����. 0�� �ε����� ������� �ʽ��ϴ�.

char map[41][41][3]; // map[0] �κа� map[x][0] �κе��� ������� �ʽ��ϴ�.
struct soldierQ *soldierQueueHead; // �ϸ��� ������� �ൿ ������ �� ����. 
int round; // ���� ���� ��ȣ


struct weak *weakTable[10]; // Ư�������� �ؽ� ���̺�


// �Լ����� ����
void mainGame();
void viewSquad();
void viewMain();
void soldierEnqueue(char input);
char soldierDequeue();
void damage(struct soldier *target, int damageVal);







void overload(int freq) // freq ���ļ��� Ư������ ����
{
	// ���ļ� ���̺��� Ư�� ���ļ��� ���� ��ο��� 3�� �������� ��
	struct weak *cursor;
	cursor = weakTable[freq];
	if (cursor == NULL) // �ش� ���ļ��� ���簡 ���� ���
	{
		printf("�ش� ���ļ��� �����Ͻ�������, �ش� ���ļ��� ���� ����� �����ϴ�.\n");
	}
	while (cursor != NULL)
	{
		if (cursor->soldierLoc->live == 1)
		{
			printf("%c�� ��� �����ϵǾ����ϴ�!\n", cursor->soldierLoc->name);
			damage(cursor->soldierLoc, 3);
		}
		cursor = cursor->next;

	}
}



void traverseBinary(int *weakArray, struct binary *cursor) // ���� ���ļ����� 2�� �˻� Ʈ���� cursor�������� �˻��ϸ�, ���� ������ weakArray �� ǥ��
{
	weakArray[cursor->value] = 1;
	if (cursor->left != NULL) // ���ʿ� child�� ������ ���. ����� ����
	{
		traverseBinary(weakArray, cursor->left);
	}
	if (cursor->right != NULL) // �����ʿ� child�� ������ ���. ����� ����
	{
		traverseBinary(weakArray, cursor->right);
	}
}


struct binary *putBinary(int input, struct binary *head) // input ���� head�� �������� 2�� �˻� Ʈ���� ����
{
	struct binary *cursor;
	cursor = head;

	// �� ���̳ʸ� ��ġ Ʈ�� ��� ����
	struct binary *current;
	current = (struct binary*)malloc(sizeof(struct binary));
	current->value = input;
	current->left = NULL;
	current->right = NULL;

	if (head == NULL) // ��Ʈ ��带 ������ ������ ��
	{
		head = current;
	}
	else // ���� ���� ��带 ������ ��ġ�� ����
	{
		while (1)
		{
			if (input < cursor->value) // ���� ��忡�� �������� �������� �� ��
			{
				if (cursor->left == NULL) // ������ ���� �ٸ� ��尡 ���� ��� -> ���⿡ ����
				{
					cursor->left = current;
					break;
				}
				else // ������ ���� �ٸ� ��尡 �ִ� ��� - �װ����� �̵�
				{
					cursor = cursor->left;
				}
			}

			else if (input > cursor->value) // ���� ��忡�� ���������� �������� �� ��
			{
				if (cursor->right == NULL) // ������ ���� �ٸ� ��尡 ���� ��� -> ���⿡ ����
				{
					cursor->right = current;
					break;
				}
				else // ������ ���� �ٸ� ��尡 �ִ� ��� - �װ����� �̵�
				{
					cursor = cursor->right;
				}
			}
		}
	}

	return head; // Ʈ���� ��� ��ġ ����
}


void massiveOverload(int input, int isBigger) // ��Ը� ������ ����, isBigger �� 0�� ��� input�̸��� ���ļ� ���, 1�� ��� input�ʰ��� ���ļ� ���
{
	int i;
	int *weakArray; // 10���� �� ���ļ� �뿪���� �ش� ���ļ��� ���� ���簡 �ִ����� 0�� 1�� ǥ��
	weakArray = (int*)malloc(sizeof(int) * 10);
	struct binary *head = NULL; // ���̳ʸ� ��ġ Ʈ���� ��Ʈ ��� ��ġ
	struct binary *cursor = NULL;




	// weakArray 0���� ��� �ʱ�ȭ
	for (i = 0; i <= 9; i++)
	{
		weakArray[i] = 0;
	}

	// �� ���ļ� �뿪���� �ش� ���ļ��� ���� ���簡 �ִ����� 0�� 1�� ǥ��
	for (i = 1; i <= 4; i++)
	{
		weakArray[squadA[i].weak] = 1;
		weakArray[squadB[i].weak] = 1;
	}


	head = putBinary(input, head); // �Է¹޾Ҵ� ���ļ� �뿪�� ���̳ʸ� ��ġ Ʈ���� ����


	// �� ������ ���Ǵ� ���ļ� �뿪���� ���̳ʸ� ��ġ Ʈ���� ����
	for (i = 0; i <= 9; i++)
	{
		if ((weakArray[i] == 1) && (i != input))
		{
			putBinary(i, head);
		}
	}



	// weakArray 0���� �ٽ� ��� �ʱ�ȭ
	for (i = 0; i <= 9; i++)
	{
		weakArray[i] = 0;
	}

	if (isBigger == 0) // �Է°����� ���� ���ļ����� ã�� ���
	{
		if (head->left != NULL)
		{
			cursor = head->left;
			traverseBinary(weakArray, cursor);
		}
	}
	else if (isBigger == 1) // �Է°����� ū ���ļ����� ã�� ���
	{
		if (head->right != NULL)
		{
			cursor = head->right;
			traverseBinary(weakArray, cursor);
		}
	}
	else
	{
		printf("isBigger ���� �߸��Ǿ����ϴ�.");
		system("pause");

	}

	// ������ ��� ���ļ� �뿪�뿡 Ư������ ����
	for (i = 0; i <= 9; i++)
	{
		if (weakArray[i] == 1)
		{
			overload(i);
		}
	}

}



void heapInsert(int input, int *workBench, int seq) // min-heap tree �� input���� �����͵��� ����� �����ϰ� ����
{
	// input �̹��� Ʈ���� �߰��� ������
	// workBench input����� heap tree �����ϴ� ���
	// seq �̹� ���� �۾��� ���� ��ȣ
	int current; // ���� �۾��ϴ� ������
	int temp;
	seq++;

	//�Է¹��� input ���� Ʈ���� ����
	workBench[seq] = input;
	current = seq;

	while (1) // min heap �� ��Ģ�� ������� ���� ������ ��� ���� �Ʒ� ���Ҹ� ��ȯ (�Ʒ�����)
	{
		if (current == 1) // �����ߴ� ���Ұ� �� ������ �ö�� ���
		{
			break;
		}

		if (workBench[current] >= workBench[current / 2]) // min heap�� ��Ģ�� ������ ���
		{
			break;
		}
		else // ���� �Ʒ��� ��ȯ�ؾ� �ϴ� ���
		{
			temp = workBench[current];
			workBench[current] = workBench[current / 2];
			workBench[current / 2] = temp;
			current = current / 2;
		}

	}
}



void heapDelete(int *workBench, int *output, int seq, int dataSize) // min-heap tree ���� �����͵��� �����ϸ� output�� ��� ����. output���� ������������ �����Ͱ� ������
{

	// workBench input���� heap tree
	// seq �̹� ���� �۾��� ���� ��ȣ
	int heapSeq = seq + 1;

	// �� Ʈ�� ������ ���� �ϳ� ������ output�� �����ϰ�, �� �ڸ��� �� ���������� ���� ���� ����
	output[dataSize - seq - 1] = workBench[1];
	workBench[1] = workBench[heapSeq];
	int current = 1;
	int temp;

	while (1) // min heap �� ��Ģ�� ������� ���� ������ ��� ���� �Ʒ� ���Ҹ� ��ȯ (������)
	{
		if ((current * 2) > heapSeq) // �� ���� �ִ� ���Ұ� ������ ������
		{
			break;
		}

		if ((workBench[current] <= workBench[current * 2])) // min heap�� ��Ģ�� �´� ���
		{
			if ((current * 2 + 1) == (heapSeq + 1)) // ���� ��ġ�� ������ �ڽ��� ���� ���
			{
				break;
			}
			else // ���� ��ġ�� ������ �ڽĵ� �ִ� ���
			{
				if (workBench[current] <= workBench[current * 2 + 1])
				{
					break;
				}
			}
		}

		// ��ȯ�ؾ� �ϴ� ���
		{
			if (workBench[current * 2] <= workBench[current * 2 + 1]) // ���� �ڽ��� ������ �ڽĺ��� �� �۰ų� ���� ���
			{
				temp = workBench[current];
				workBench[current] = workBench[current * 2];
				workBench[current * 2] = temp;
				current *= 2;
			}

			else // ������ �ڽ��� ���� �ڽĺ��� �� ���� ���
			{
				if ((current * 2 + 1) != (heapSeq + 1)) // ���� ��ġ�� ������ �ڽ��� �ִ� ���
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



int *heapSort(int *input, int dataSize) // heap sort ����
{



	int *output;
	int *workBench; // workBench �迭�� �ε����� 1���� �����(ȥ�� ����)
	int i;


	workBench = (int*)malloc(sizeof(int) * (dataSize + 2));
	output = (int*)malloc(sizeof(int) * dataSize);
	if ((output == NULL) || (workBench == NULL))
	{
		printf("Memory allocation failed. Please restart this program\n");
		getchar();
	}


	for (i = 0; i < dataSize; i++) // input�� �� ������ ������� �� Ʈ���� �߰�
	{
		heapInsert(input[i], workBench, i);
	}



	for (i = (dataSize - 1); i >= 0; i--)
	{
		heapDelete(workBench, output, i, dataSize); // �� Ʈ���� �� ������ output�� �߰�. output���� ������ ������������ ���ĵǰ� ��.
	}




	free(input);

	free(workBench);

	return output;

}



void resetTime() // ���� ü���� - �ð� ���� ���� - ���� ���� �ִ� ���� �ൿ ť�� �ٽ� ������ ����
{
	int i, j;

	int *dexQueue; // ������ ��ø �ɷ�ġ���� ���Ե�
	dexQueue = (int*)malloc(sizeof(int) * 8);
	// ������ �� ��ø �ɷ�ġ �ڷ� ����
	for (i = 0; i <= 3; i++)
	{
		dexQueue[i] = squadA[i + 1].DEX;
		dexQueue[i + 4] = squadB[i + 1].DEX;
	}

	dexQueue = heapSort(dexQueue, 8); // �ڷ���� heap sort

	while (soldierQueueHead != NULL) // ���� �ൿ ť�� ���� ��� ����
	{
		soldierDequeue();
	}


	


	// �� ��ø �ɷ�ġ�� �ش��ϴ� ������� ���ĵ� ��� �ൿ ť�� Enqueue
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


void stackPush(struct soldier *current) // ������ źâ�� źȯ �߰� �۾�
{
	current->stackTop++;
	if (current->stackTop > 4)
	{
		printf("���� - źȯ ���� �ʰ��Ǿ����ϴ�.\n");
		system("pause");
	}
}


void stackPop(struct soldier *current) // ������ ��� �� źâ���� źȯ�� ���� �۾�
{
	current->stackTop--;
	if (current->stackTop <= -2)
	{
		printf("���� - ���� źȯ ���� �����Դϴ�.\n");
		system("pause");
	}
}


void weakAdd(struct soldier *input) // ���ļ� �ؽ� ���̺� input���� ���� ���縦 ������ �ε����� ����
{
	int index = input->weak;

	// ���� ���� ������ Ű�� �� ����
	struct weak *currentCell;
	currentCell = (struct weak*)malloc(sizeof(struct weak));
	currentCell->soldierLoc = input;
	currentCell->next = NULL;

	if (weakTable[index] == NULL) // �ش� �ε����� �� ó�� ������ ���
	{
		weakTable[index] = currentCell;
	}
	else // �ش� �ε����� �ٸ� ���Ұ� �̹� ���� ��� - overflow chaining
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



void soldierEnqueue(char input) // �Է¹��� �̸��� ���縦 �ൿ ���� ť �� �ڿ� ����
{
	if (soldierQueueHead == NULL) // ť�� ���Ұ� ���� ���
	{
		struct soldierQ *element;
		element = (struct soldierQ*)malloc(sizeof(struct soldierQ));
		element->name = input;
		element->next = NULL;
		soldierQueueHead = element;

	}
	else // ť�� �ٸ� ���Ұ� �ִ� ��� - ť �� �ڿ� ����
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


char soldierDequeue() // �ൿ ť �� ���� ���縦 ť���� ���� (�ַ� ���� ������ �ѱ� �� ���)
{
	int output;
	struct soldierQ *temp;
	if (soldierQueueHead->next == NULL) // ���� �ൿ ť�� ���Ұ� �ϳ����� ����� ����
	{
		output = soldierQueueHead->name;

		free(soldierQueueHead);
		soldierQueueHead = NULL;
	}
	else // ���� �ൿ ť�� �ٸ� ���Ұ� �ִ� ����� ����
	{
		output = soldierQueueHead->name;
		temp = soldierQueueHead->next;
		free(soldierQueueHead);
		soldierQueueHead = temp;
	}


	return output;
}



void merge(int *input, int *workBench, int start, int end) // �� ĭ���� ��������� merge�ϴ� �Լ�, ���� �׷���� ū �׷��� ������ ���� �����Ѵ�
{
	// input�� ������ ���� �ڷ��
	// workBench�� �ӽ� �۾� ���. ������ workBench ���� ������ ������ input���� ���ĵ� �ڷḦ �ű��.
	// start ���� pass�� merge �Լ��� ������ input �迭 ���� ���� ����
	// end ���� pass�� merge �Լ��� ������ input �迭 ���� �� ����
	int i, j, k;
	int temp;

	if (((end - start) != 1) && ((end - start) != 2)) // ���� �н��� �ڷ� ������ 0���� 1���� �ƴ϶��, ��������� ���� �׷�� ���� �׷��� merge ����
	{
		merge(input, workBench, start, ((start + end) / 2)); // ���� �׷��� merge ����
		merge(input, workBench, ((start + end) / 2) + 1, end); // ���� �׷��� merge ����
	}

	if ((end - start) == 2) // ���� ������� �׷��� ���Ұ� 3���� ���, �ļ� ������ ���� �̸� ù°�� ��° ���Ҹ� ������ ���´�.
	{
		if (input[start] > input[start + 1])
		{
			temp = input[start];
			input[start] = input[start + 1];
			input[start + 1] = temp;
		}
	}

	// ���� ���� �׷���� ����� merge �Լ� ó���� �Ϸ�Ǹ�(���� �׷���� ������ �Ϸ�Ǹ�), ���� �׷��� ���� ����
	// �� �׷�� �� �׷��� �� �� ���� ���� �� ������ ���ϰ�, �� �� ���� ���� workBench �� �ִ� ���� �ݺ��Ѵ�.
	i = start; // ���� �׷��� Ŀ��
	j = ((start + end) / 2) + 1; // ���� �׷��� Ŀ��
	k = start;
	while (1)
	{


		if (input[i] < input[j]) // ���� �׷��� �� �� ���� �� ����
		{
			workBench[k] = input[i];
			i++;
		}
		else // ���� �׷��� �� �� ���� �� ����
		{
			workBench[k] = input[j];
			j++;
		}
		k++;


		if (i == (((start + end) / 2) + 1)) // ���� �׷��� ������ ��� ������ ���
		{
			while (k != (end + 1))
			{
				workBench[k] = input[j];
				j++;
				k++;
			}
			break;
		}
		else if (j == end + 1) // ���� �׷��� ������ ��� ������ ���
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


	for (i = start; i <= end; i++) // workBench���� �۾��� ������� input���� �ű�
	{
		input[i] = workBench[i];
	}
}



int *mergeSort(int *input, int dataSize) // merge sort ����
{

	int *workBench; // ���� �۾��� ����ϴ� �ӽ� ����


	workBench = (int*)malloc(sizeof(int) * dataSize);
	if (workBench == NULL)
	{
		printf("Memory allocation failed. Please restart this program\n");
		system("pause");
	}


	merge(input, workBench, 0, (dataSize - 1)); // ��������� ���� ���� �׷쿡�� ū �׷� ������ �׷���� �����ϴ� �Լ�


	free(workBench);

	return input;

}



void rocket(int targetSide) // ���� ü���� - ���� ��ó ����. Trie ������ targetSide ��и��� ��� ������ �ı�
{
	char **trieHead[5]; // Trie�� ���. 0�� �ε����� ������� �ʽ��ϴ�.
	int i, j;

	// �� ��и� �� �ش��ϴ� map �����͸� ���� ���� �Ҵ�
	trieHead[1] = (char**)malloc(sizeof(char*) * 400);
	trieHead[2] = (char**)malloc(sizeof(char*) * 400);
	trieHead[3] = (char**)malloc(sizeof(char*) * 400);
	trieHead[4] = (char**)malloc(sizeof(char*) * 400);
	// �迭�� ä��� �� ����ϴ� ī����
	int counter1 = 0;
	int counter2 = 0;
	int counter3 = 0;
	int counter4 = 0;

	// map�� �ּҵ��� ���� �´� ��и鿡 ����
	for (i = 1; i <= 40; i++)
	{
		for (j = 1; j <= 40; j++)
		{
			if (i <= 20 && j <= 20) // �ش� ��ǥ�� 1��и��� ���
			{
				trieHead[1][counter1] = map[i][j];
				counter1++;
			}
			else if (i <= 20 && j > 20) // �ش� ��ǥ�� 2��и��� ���
			{
				trieHead[2][counter2] = map[i][j];
				counter2++;
			}
			else if (i > 20 && j > 20) // �ش� ��ǥ�� 3��и��� ���
			{
				trieHead[3][counter3] = map[i][j];
				counter3++;
			}
			else if (i > 20 && j <= 20) // �ش� ��ǥ�� 4��и��� ���
			{
				trieHead[4][counter4] = map[i][j];
				counter4++;
			}
		}
	}

	// ���� �ı� �۾�
	for (i = 0; i <= 399; i++)
	{
		if ((strcmp(trieHead[targetSide][i], "��") == 0) || (strcmp(trieHead[targetSide][i], "��") == 0)) // Ÿ�� ��и��� �ش� ��ġ�� ������ ���, �ı�
		{
			strcpy_s(trieHead[targetSide][i], sizeof("��"), "��");
		}
	}

}



void viewMap() // ���� ǥ���ϴ� �Լ�
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




void nextTurn() // ���� ������ �����ϴ� �� ����ϴ� �Լ�
{
	

	// ���� ť�� �� ���� ���縦 deQueue�ϰ� �� ť�� �� �ڿ� �ٽ� enQueue ����
	soldierEnqueue(soldierDequeue());


	mainGame(); // ���� ȭ������ ���ư� -> ���� ��
}



void damage(struct soldier *target, int damageVal) // ���ظ� �ִ� �� ����ϴ� �Լ�. target�� ü���� damageVal ��ŭ ���ҽ�Ű�� ��� ����
{
	target->curHP -= damageVal;

	printf("%c�� %d�� �������� �Ծ����ϴ�.\n", target->name, damageVal);

	if (target->curHP < 0) // Ÿ�� ��� ��
	{
		target->live = 0;
		printf("%c�� HP�� �����Ǿ� ���������ϴ�!\n", target->name);

		//������ġ �����
		strcpy_s(map[target->location[0]][target->location[1]], sizeof("��"), "��");
	}



}



void mainGame() // ���� ���� ������ ���� ȭ��
{

	system("cls");


	// ���� ����
	if (squadA[1].live == 0 && squadA[2].live == 0 && squadA[3].live == 0 && squadA[4].live == 0)
	{
		printf("�����մϴ�! B ���� �¸��߽��ϴ�!\n");
		system("pause");




		viewMain();
	}
	if (squadB[1].live == 0 && squadB[2].live == 0 && squadB[3].live == 0 && squadB[4].live == 0)
	{
		printf("�����մϴ�! A ���� �¸��߽��ϴ�!\n");
		system("pause");




		viewMain();
	}


	

	int choice; // �̹� ���� �ൿ ���ÿ� ����ϴ� ����
	int i, j;
	struct soldier *current; // �̹� �Ͽ� �ൿ�ϰ� �ִ� ����
	
	// �̹� �Ͽ� �ൿ�ϴ� ���� �����͸� current �� �ҷ���
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

	// �ҷ��� ���簡 �̹� ����� ������ ���, ���� ������ ������ �ٷ� ����
	if (current->live == 0)
	{
		nextTurn();
	}

	// ���� ���� ȭ�� ���
	viewMap(); // �� ���
	printf("\n\n");
	printf("%d ����\n", round);
	printf("%c�� ���Դϴ�.\n", current->name);
	printf("1. �̵� 2. ��� 3. Ư������ 4. ���� ���� Ȯ�� 5. ������ 6. ����ü����\n -> ");
	scanf_s("%d", &choice);
	while (choice != 1 && choice != 2 && choice != 3 && choice != 4 && choice != 5 && choice != 6) // �߸��� �ɼ� ���� ���
	{
		printf("�߸��� �ɼ��Դϴ�. �ùٸ� �ൿ�� ������ �ּ���\n");
		printf("1. �̵� 2. ��� 3. Ư������ 4. ���� ���� Ȯ�� 5. ������ 6. ����ü����\n -> ");
		scanf_s("%d", &choice);
	}

	if (choice == 1) // �̵�
	{
		int moveX; // �̵��� x�� ��
		int moveY; // �̵��� y�� ��
		int afterX;
		int afterY;

		while (1)
		{
			printf("�̵��� x�� ���� �Է��� �ּ��� (-5~5)\n");
			scanf_s("%d", &moveX);
			while (moveX < -5 || moveX > 5)
			{
				printf("�ùٸ� ���� �Է��� �ּ���.\n");
				scanf_s("%d", &moveX);
			}
			printf("�̵��� y�� ���� �Է��� �ּ��� (-5~5)\n");
			scanf_s("%d", &moveY);
			while (moveY < -5 || moveY > 5)
			{
				printf("�ùٸ� ���� �Է��� �ּ���.\n");
				scanf_s("%d", &moveY);
			}




			afterX = current->location[0] - moveY;
			afterY = current->location[1] + moveX;

			// �̵��Ϸ��� �ϴ� ���� �� ĭ�� �ƴ� ���
			if (afterX > 40 || afterY > 40 || afterX < 0 || afterX < 0 || strcmp(map[afterX][afterY], "��") != 0)
			{
				printf("�װ����δ� �̵��� �� �����ϴ�.\n");
				continue;
			}
			// �ش� ĭ���� �̵� ������ ���, �̵��ϰ� ��ġ ���� �۾� ����
			else
			{
				// �� ����
				strcpy_s(map[current->location[0]][current->location[1]], sizeof("��"), "��");

				// ������ ��ġ ���� ����
				current->location[1] += moveX;
				current->location[0] -= moveY;

				// �� ����
				if (current->name == '1')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}
				else if (current->name == '2')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}
				else if (current->name == '3')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}
				else if (current->name == '4')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}
				else if (current->name == 'A')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}
				else if (current->name == 'B')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}
				else if (current->name == 'C')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}
				else if (current->name == 'D')
				{
					strcpy_s(map[afterX][afterY], sizeof("��"), "��");
					break;
				}



				break;
			}
		}
	}
	else if (choice == 2) // ���
	{
		char targetN; // ��� ����� �̸�
		struct soldier *target = NULL; // ��� ����� ����

		// ��ź�� Ȯ��
		if (current->stackTop == -1) // źâ�� �� ��� - ��� �Ұ�
		{
			printf("źâ�� ������ϴ�. �������� �ؾ� ����� �����մϴ�.\n");
			mainGame();
		}

		while (1)
		{
			printf("��� ����� �Է��ϼ���(���� �Ǵ� �빮��)\n");
			printf("1 2 3 4 A B C D\n -> ");
			scanf_s("%c", &targetN, sizeof(char));
			scanf_s("%c", &targetN, sizeof(char));
			if (targetN != '1' && targetN != '2' && targetN != '3' && targetN != '4' && targetN != 'A' && targetN != 'B' && targetN != 'C' && targetN != 'D')
			{
				printf("�߸��� �Է��Դϴ�.\n");
				continue;
			}

			// �Է¹��� Ÿ�� �̸����� Ÿ���� ������ �ҷ���
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



			if (current->name == targetN) // �ڽ��� ��� �ϴ� ���
			{
				printf("����� ������ �����Դϴ�.\n");
			}
			else if (current->team == target->team) // �Ʊ��� ��� �ϴ� ���
			{
				printf("�Ʊ��� ��� ���� ���� �ʽ��ϴ�.\n");
			}
			else if (target->live == 0) // �̹� ����� �ο��� ����Ϸ��� ���
			{
				printf("Ȯ�λ���� ���׹� ���࿡ ��߳��ϴ�.\n");
			}
			else // ��� ����� valid �� ���
			{
				break;
			}
		}



		int covered; // ��� ����� ������ ����

		// ���� ���� �۾�
		if (current->location[1] > target->location[1] && current->location[0] > target->location[0]) // ���� �� 1��и鿡 ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0]][target->location[1] + 1], "��") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0]][target->location[1] + 1], "��") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] < target->location[1] && current->location[0] > target->location[0]) // ���� �� 2��и鿡 ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0]][target->location[1] - 1], "��") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0]][target->location[1] - 1], "��") == 0 || strcmp(map[target->location[0] + 1][target->location[1]], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] < target->location[1] && current->location[0] < target->location[0]) // ���� �� 3��и鿡 ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0]][target->location[1] - 1], "��") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0]][target->location[1] - 1], "��") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] > target->location[1] && current->location[0] < target->location[0]) // ���� �� 4��и鿡 ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0]][target->location[1] + 1], "��") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0]][target->location[1] + 1], "��") == 0 || strcmp(map[target->location[0] - 1][target->location[1]], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] < target->location[1] && current->location[0] == target->location[0]) // ���� �ٷ� ���ʿ� ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0]][target->location[1] - 1], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0]][target->location[1] - 1], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] > target->location[1] && current->location[0] == target->location[0]) // ���� �ٷ� ���ʿ� ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0]][target->location[1] + 1], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0]][target->location[1] + 1], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] == target->location[1] && current->location[0] < target->location[0]) // ���� �ٷ� ���ʿ� ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0] - 1][target->location[1]], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0] - 1][target->location[1]], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
			else
			{
				covered = 0;
			}
		}
		else if (current->location[1] == target->location[1] && current->location[0] > target->location[0]) // ���� �ٷ� ���ʿ� ��ġ
		{
			// ���� ���� ���
			if (strcmp(map[target->location[0] + 1][target->location[1]], "��") == 0)
			{
				covered = 2;
			}
			// �ݾ��� ���
			else if (strcmp(map[target->location[0] + 1][target->location[1]], "��") == 0)
			{
				covered = 1;
			}
			// ������ ���
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

		// ���߷� ���
		int aim = current->aim - (covered * 20);


		printf("�߻��մϴ�...\n");
		printf("������ ���� +%d%%\n", current->aim);
		if (covered == 1) // �� �ݾ��� ���
		{
			printf("�� �ݾ��� -20%%\n");
		}
		else if (covered == 2) // �� ���� ���� ���
		{
			printf("�� ���� ���� -40%%\n");
		}
		printf("-���߷� %d%%\n", aim);

		int crit = 10;
		if (covered == 0) // �� ������ ���
		{
			crit += 40;
		}

		printf("�⺻ ġ��Ÿ�� +10%%\n");
		if (covered == 0) // �� ������ ���
		{
			printf("�� ������ +40%%\n");
		}
		printf("-ġ��Ÿ�� %d%%\n", crit);
		printf("���͸� ���� �߻��մϴ�.\n");
		system("pause");



		// ����� ���� ���ο� ġ��Ÿ ���� ����
		int luck = rand() % 101;
		int isCrit; // ġ��Ÿ ���θ� 0�� 1�� ��Ÿ��
		if (crit >= luck) // ġ��Ÿ ����
		{
			isCrit = 1;
		}
		else // ġ��Ÿ�� �ƴ� ���
		{
			isCrit = 0;
		}

		int damageVal = current->ATK; // ���� ������
		luck = rand() % 101;


		if (aim >= luck)
		{
			//����
			if (isCrit == 1) // ġ��Ÿ
			{
				printf("ġ��Ÿ!\n");
				damageVal *= 2;
			}
			else // �ܼ� ����
			{
				printf("����!\n");
			}

			damage(target, damageVal);


		}
		else
		{
			//������
			printf("������!\n");
		}

		// ��ź�� �Һ� ���
		current->stackTop--;

		
		system("pause");

	}
	else if (choice == 3) // Ư������
	{
		int freq; // Ư�������� ������ ���ļ�

		printf("�ش��ϴ� ���ļ� �뿪�� ������ ������ �����մϴ�...\n");
		printf("�����Ͻ�ų ���ļ� �뿪�� �Է��ϼ��� (0~9)\n");
		scanf_s("%d", &freq);
		while (0 > freq || freq > 9)
		{
			printf("�߸��� ���ļ� �뿪�Դϴ�.\n");
			printf("�����Ͻ�ų ���ļ� �뿪�� �Է��ϼ��� (0~9)\n");
			scanf_s("%d", &freq);
		}

		overload(freq); // Ư������ ����
		
		system("pause");
	}
	else if (choice == 4) // ��� ���� Ȯ��
	{
		viewSquad();
	}

	else if (choice == 5) // ������ - ź���� �� �� ������ ź�� �߰�
	{
		printf("�ѱ��� ź���� ä�����ϴ�!\n");
		while (current->stackTop != 4)
		{
			stackPush(current);
		}
		system("pause");
	}

	else if (choice == 6) // ���� ü����(���� ��ų) �ߵ�
	{
		int randChoice;
		randChoice = rand() % 3;

		if (randChoice == 0) // ���� ü���� - �ð� ���� �ߵ�
		{
			printf("�ð� ���� �ߵ� - ���� �� ó�� �������� �ٽ� ���۵˴ϴ�.\n");
			resetTime(); // �ð� ���� ����
			system("pause");
			mainGame(); 
		}
		else if (randChoice == 1) // ���� ü���� - ���� ������ �ߵ�
		{
			int freq; // ��� ���ļ�
			int massOption; // ��� ���ļ����� ū/���� ���� ���� ����

			printf("���� ������ �ߵ� - Ư�� ���ļ����� �۰ų� ū ��� ���ļ��� �����Ͻ�ŵ�ϴ�.\n");
			printf("�����Ͻ�ų ���� ���ļ� �뿪�� �Է��ϼ��� (0~9)\n");
			scanf_s("%d", &freq);
			while (0 > freq || freq > 9)
			{
				printf("�߸��� ���ļ� �뿪�Դϴ�.\n");
				printf("�����Ͻ�ų ���ļ� �뿪�� �Է��ϼ��� (0~9)\n");
				scanf_s("%d", &freq);
			}

			printf("���� ���ļ����� ū ���ļ����� �����Ϸ��� 1��, ���� ���ļ����� �����Ϸ��� 0�� �Է��ϼ���\n");
			scanf_s("%d", &massOption);
			while (massOption != 0 && massOption != 1)
			{
				printf("�߸��� ���ļ� �뿪�Դϴ�.\n");
				printf("���� ���ļ����� ū ���ļ����� �����Ϸ��� 1��, ���� ���ļ����� �����Ϸ��� 0�� �Է��ϼ���\n");
				scanf_s("%d", &massOption);

			}

			massiveOverload(freq, massOption); // �ش� �Ķ���ͷ� ���� ������ ����
			system("pause");
		}
		else if (randChoice == 2) // ���� ü���� - ���� ��ó �߻�
		{
			int targetSide; // ��� ��и�

			printf("���� ��ó ��� - Ư�� ��и��� ��� ���󹰵��� �ı��մϴ�.\n");
			printf("���󹰵��� �ı��� ��и��� �Է��� �ּ���.\n");
			scanf_s("%d", &targetSide);
			while (targetSide != 1 && targetSide != 2 && targetSide != 3 && targetSide != 4)
			{
				printf("�߸��� �Է°��Դϴ�.\n");
				printf("���󹰵��� �ı��� ��и��� �Է��� �ּ���.\n");
				scanf_s("%d", &targetSide);
			}

			rocket(targetSide); // ���� ��ó �߻� ����
			printf("�ش� ��и��� ���󹰵��� ���߷� ��� �ı��Ǿ����ϴ�!\n");
			system("pause");
		}
		
	}

	nextTurn(); // �̹� ������ �ൿ�� ����. ���� ������ ������ �Ѿ
}



void viewSquad() // ����� ���� ����
{
	int i;


	printf("\n���� ����\n\n");
	printf("�̸� �ִ�HP ����HP ���� ȭ�� ��ø ���ļ� ��ź��\n");
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




void initSoldier(struct soldier *squad) // �Է¹��� ������ �ɷ�ġ�� �������� �ʱ�ȭ
{
	/*
	HP�� ���� -> 15~20
	������ ���� -> 50~80
	ȭ���� ���� -> 4~6
	��ø�� ���� -> 0~99
	������ ���� -> 0~9
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



void initGame() // ���� ���۽ÿ� �� ���� �ʱ�ȭ, �� �ҷ�����, ���� ��ġ �۾�
{
	// �� �� ���� �ʱ�ȭ�� ����
	while (soldierQueueHead != NULL) // ���� �ൿ ť�� ���� ��� ����
	{
		soldierDequeue();
	}

	system("cls");



	int i, j;
	char temp;

	// ���ļ� �ؽ� ���̺� �ʱ�ȭ
	for (i = 0; i <= 9; i++)
	{
		weakTable[i] = NULL;
	}


	round = 1;

	srand((unsigned)time(NULL));


	// �� ���� �о��
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

	// ������� �ɷ�ġ �ʱ�ȭ
	int isOverLap;
	while (1)
	{
		isOverLap = 0;
		for (i = 1; i <= 4; i++)
		{
			initSoldier(&squadA[i]);
			initSoldier(&squadB[i]);
		}

		// ��ø �ɷ�ġ�� �ߺ� ���� �˻� - �ߺ��� �ɷ�ġ �缳��
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

	// ���ļ� �ؽ� ���̺� ������� ���ļ� ����
	for (i = 1; i <= 4; i++)
	{
		weakAdd(&squadA[i]);
		weakAdd(&squadB[i]);
	}

	// ������� �̸�/�� �ʱ�ȭ
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





	// ��ø �ɷ�ġ�� merge sort�� �̿��� ����
	int *dexQueue;
	dexQueue = (int*)malloc(sizeof(int) * 8);
	// ������ �ڷ� ����
	for (i = 0; i <= 3; i++)
	{
		dexQueue[i] = squadA[i + 1].DEX;
		dexQueue[i + 4] = squadB[i + 1].DEX;
	}


	dexQueue = mergeSort(dexQueue, 8);

	// ���ĵ� ��ø �ɷ�ġ�� �ش��ϴ� ������� �ൿ ť�� ������� Enqueue ��
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


	// ������� �ʿ� �����ϰ� ��ġ
	for (i = 1; i <= 4; i++)
	{
		while (1)
		{
			squadA[i].location[0] = rand() % 40 + 1;
			squadA[i].location[1] = rand() % 40 + 1;
			if (strcmp(map[squadA[i].location[0]][squadA[i].location[1]], "��") == 0)
			{
				if (i == 1)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("��"), "��");
					break;
				}
				else if (i == 2)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("��"), "��");
					break;
				}
				else if (i == 3)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("��"), "��");
					break;
				}
				else if (i == 4)
				{
					strcpy_s(map[squadA[i].location[0]][squadA[i].location[1]], sizeof("��"), "��");
					break;
				}

			}
		}

	}




	// ������� �ʿ� �����ϰ� ��ġ
	for (i = 1; i <= 4; i++)
	{
		while (1)
		{
			squadB[i].location[0] = rand() % 40 + 1;
			squadB[i].location[1] = rand() % 40 + 1;
			if (strcmp(map[squadB[i].location[0]][squadB[i].location[1]], "��") == 0)
			{
				if (i == 1)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("��"), "��");
					break;
				}
				else if (i == 2)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("��"), "��");
					break;
				}
				else if (i == 3)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("��"), "��");
					break;
				}
				else if (i == 4)
				{
					strcpy_s(map[squadB[i].location[0]][squadB[i].location[1]], sizeof("��"), "��");
					break;
				}

			}
		}

	}



	viewSquad(); // ���� ���� �� ������� ������ �����ִ� â���� ����
}



void viewRule() // ��� ���� - rule.txt�� �ҷ��� - �� �� �ٽ� ���� �޴���
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




void viewMain() // ���� ȭ���� ǥ���ϰ�, �ش��ϴ� �޴��� �̵��մϴ�.
{
	system("cls");

	int choice; // ���ð��� �Է¹���

	printf("  ��      ��       ������  ������  ��       ��\n");
	printf("   ��    ��        ��          ��      ��  ���   ���\n");
	printf("    ��  ��         ��          ��      ��  �� �� �� ��\n");
	printf("     ���          ��          ��      ��  ��  ��   ��\n");
	printf("      ��   ����  ��          ��      ��  ��  ��   ��\n");
	printf("     ���          ��          ��      ��  ��  ��   ��\n");
	printf("    ��  ��         ��          ��      ��  ��  ��   ��\n");
	printf("   ��    ��        ��          ��      ��  ��  ��   ��\n");
	printf("  ��      ��       ������  ������  ��  ��   ��\n\n\n");
	printf("X-COM CLI Edition - Developed by Gazua team\n\n\n");

	printf("�޴��� �����ϼ���\n\n");
	printf("1. ���� ���� 2. ���� ��Ģ ���� 3. ������\n");
	printf("-> ");
	scanf_s("%d", &choice);
	while (choice != 1 && choice != 2 && choice != 3) // �߸��� �ɼ� ���� ���
	{
		printf("�߸��� �ɼ��Դϴ�. �ùٸ� �޴��� ������ �ּ���\n");
		printf("1. ���� ���� 2. ���� ��Ģ ���� 3. ������\n");
		printf("-> ");
		scanf_s("%d", &choice);
	}

	if (choice == 1) // ���� ����
	{
		initGame();
	}
	else if (choice == 2) // ��� ����
	{
		viewRule();
	}
	else if (choice == 3) // ���� ����
	{
		exit(0);
	}
}



void main()
{
	viewMain(); // ���� ���� ȭ������ �̵�

}
#include <stdio.h>
#include <stdlib.h>

#define PEASANT 0x08
#define WOLF	0x04
#define GOAT	0x02
#define CABBAGE	0x01

// 주어진 상태 state의 이름(마지막 4비트)을 화면에 출력
// 예) state가 7(0111)일 때, "<0111>"을 출력
static void print_statename( FILE *fp, int state);

// 주어진 상태 state에서 농부, 늑대, 염소, 양배추의 상태를 각각 추출하여 p, w, g, c에 저장
// 예) state가 7(0111)일 때, p = 0, w = 1, g = 1, c = 1
static void get_pwgc( int state, int *p, int *w, int *g, int *c);

// 허용되지 않는 상태인지 검사
// 예) 농부없이 늑대와 염소가 같이 있는 경우 / 농부없이 염소와 양배추가 같이 있는 경우
// return value: 1 허용되지 않는 상태인 경우, 0 허용되는 상태인 경우
static int is_dead_end( int state);

// state1 상태에서 state2 상태로의 전이 가능성 점검
// 농부 또는 농부와 다른 하나의 아이템이 강 반대편으로 이동할 수 있는 상태만 허용
// 허용되지 않는 상태(dead-end)로의 전이인지 검사
// return value: 1 전이 가능한 경우, 0 전이 불이가능한 경우 
static int is_possible_transition( int state1,	int state2);

// 상태 변경: 농부 이동
// return value : 새로운 상태
static int changeP( int state);

// 상태 변경: 농부, 늑대 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePW( int state);

// 상태 변경: 농부, 염소 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePG( int state);

// 상태 변경: 농부, 양배추 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1 
static int changePC( int state);

// 주어진 state가 이미 방문한 상태인지 검사
// return value : 1 visited, 0 not visited
static int is_visited( int visited[], int level, int state);

// 방문한 상태들을 차례로 화면에 출력
static void print_states( int visited[], int count);

// recursive function
static void dfs_main( int state, int goal_state, int level, int visited[]);

////////////////////////////////////////////////////////////////////////////////
// 상태들의 인접 행렬을 구하여 graph에 저장
// 상태간 전이 가능성 점검
// 허용되지 않는 상태인지 점검 
void make_adjacency_matrix( int graph[][16]);

// 인접행렬로 표현된 graph를 화면에 출력
void print_graph( int graph[][16], int num);

// 주어진 그래프(graph)를 .net 파일로 저장
// pgwc.net 참조
void save_graph( char *filename, int graph[][16], int num);

////////////////////////////////////////////////////////////////////////////////
// 깊이 우선 탐색 (초기 상태 -> 목적 상태)
void depth_first_search( int init_state, int goal_state)
{
	int level = 0;
	int visited[16] = {0,}; // 방문한 정점을 저장
	
	dfs_main( init_state, goal_state, level, visited); 
}

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char **argv)
{
	int graph[16][16] = {0,};
	
	// 인접 행렬 만들기
	make_adjacency_matrix( graph);

	// 인접 행렬 출력 (only for debugging)
	//print_graph( graph, 16);
	
	// .net 파일 만들기
	save_graph( "pwgc.net", graph, 16);

	// 깊이 우선 탐색
	depth_first_search( 0, 15); // initial state, goal state
	
	return 0;
}

static void print_statename( FILE *fp, int state)
{
	char bits[5];
	int p, w, g, c;
	get_pwgc(state, &p, &w, &g, &c);
	bits[0] = p + '0';
	bits[1] = w + '0';
	bits[2] = g + '0';
	bits[3] = c + '0';
	bits[4] = '\0';
	
	fprintf(fp, "<%s>", bits);
}

static void get_pwgc( int state, int *p, int *w, int *g, int *c)
{
	// (8 & 0111) = 0000

	*p = (8 & state) >> 3;
	*w = (4 & state) >> 2;
	*g = (2 & state) >> 1;
	*c = (1 & state);
}

static int is_dead_end( int state)
{
	int p, w, g, c;
	get_pwgc(state, &p, &w, &g, &c);

	if (p != g && (w == g || g == c)) return 1; // dead end인 경우 
	else return 0;
}

static int is_possible_transition( int state1,	int state2)
{
	int p1, w1, g1, c1; // state1
	int p2, w2, g2, c2; // state2

	get_pwgc(state1, &p1, &w1, &g1, &c1);
	get_pwgc(state2, &p2, &w2, &g2, &c2);

	if (is_dead_end(state2)) return 0; // dead end인경우 not possible

	if (p1 == p2) return 0; // 농부가 같이 안 움직인 경우
	if (p1 != w1 && w1 != w2) return 0; // 농부 없이 w 이동
	if (p1 != g1 && g1 != g2) return 0; // 농부 없이 g 이동
	if (p1 != c1 && c1 != c2) return 0; // 농부 없이 c 이동

	// 둘 이상 이동
	if (w1 != w2 && g1 != g2) return 0;
	if (w1 != w2 && c1 != c2) return 0;
	if (g1 != g2 && c1 != c2) return 0;

	return 1; // 가능한 전이
}

static int changeP( int state)
{
	// if (state & 8) return state &= ~8; // p = 1이면 p 비트 끔
	// else return state |= 8; // p = 0이면 p 비트 킴

	return (state & 8) ? (state &= ~8) : (state |= 8);
}

static int changePW( int state)
{
	int state2 = state;

	(state & 8) ? (state2 &= ~8) : (state2 |= 8); // p 이동
	(state & 4) ? (state2 &= ~4) : (state2 |= 4); // w 이동
	if (is_possible_transition(state, state2)) return state2;
	else return -1;
}

static int changePG( int state)
{
	int state2 = state;

	(state & 8) ? (state2 &= ~8) : (state2 |= 8); // p 이동
	(state & 2) ? (state2 &= ~2) : (state2 |= 2); // g 이동
	if (is_possible_transition(state, state2)) return state2;
	else return -1;
}

static int changePC( int state)
{
	int state2 = state;

	(state & 8) ? (state2 &= ~8) : (state2 |= 8); // p 이동
	(state & 1) ? (state2 &= ~1) : (state2 |= 1); // c 이동
	if (is_possible_transition(state, state2)) return state2;
	else return -1;
}

static int is_visited( int visited[], int level, int state)
{
	for (int i = 0; i < level; i++) 
		if (visited[i] == state) return 1;
	
	return 0;
}

static void print_states( int visited[], int count)
{
	for (int i = 0; i < count; i++)
	{
		print_statename(stdout, visited[i]);
		printf("\n");
	}
}

static void dfs_main( int state, int goal_state, int level, int visited[])
{
	// cur state 출력
	printf("cur state is ");
	print_statename(stdout, state);
	printf(" (level %d)\n", level);

	visited[level] = state; // 방문 표시 

	// 전이가 모두 성공했을 때
	if (state == goal_state)
	{
		printf("Goal-state found!\n");
		print_states(visited, level + 1);
		printf("\n");
		visited[level] = 0;
		return;
	}

	int condition; // p, pw, pg, pc 순서로 변경하며 가능한 전이인지 판별

	// p
	condition = changeP(state);
	if (is_dead_end(condition))
	{
		printf("	next state ");
		print_statename(stdout, condition);
		printf(" is dead-end\n");
	}
	else if (is_visited(visited, level, condition))
	{
		printf("	next state ");
		print_statename(stdout, condition);
		printf(" has been visited\n");
	}
	else
	{
		dfs_main(condition, goal_state, level + 1, visited);
		printf("back to ");
		print_statename(stdout, state);
		printf(" (level %d)\n", level);
	}

	// pw
	condition = changePW(state);
	if((state & 8) >> 3 == (state & 4) >> 2)
	{
		if (condition != -1)
		{
			if (is_visited(visited, level, condition))
			{
				printf("	next state ");
				print_statename(stdout, condition);
				printf(" has been visited\n");
			}
			else 
			{
				dfs_main(condition, goal_state, level + 1, visited);
				printf("back to ");
				print_statename(stdout, state);
				printf(" (level %d)\n", level);
			}
		}
		else
		{
			condition = state;	
			(state & 8) ? (condition &= ~8) : (condition |= 8); 
			(state & 4) ? (condition &= ~4) : (condition |= 4);
			if(is_dead_end(condition))
			{
				printf("	next state ");
				print_statename(stdout, condition);
				printf(" is dead-end\n");
			}			
		}
	}

	// pg
	condition = changePG(state);
	if((state & 8) >> 3 == (state & 2) >> 1)
	{
		if (condition != -1)
		{
			if (is_visited(visited, level, condition))
			{
				printf("	next state ");
				print_statename(stdout, condition);
				printf(" has been visited\n");
			}
			else 
			{
				dfs_main(condition, goal_state, level + 1, visited);
				printf("back to ");
				print_statename(stdout, state);
				printf(" (level %d)\n", level);
			}	
		}
		else
		{
			condition = state;	
			(state & 8) ? (condition &= ~8) : (condition |= 8); 
			(state & 2) ? (condition &= ~2) : (condition |= 2);
			if (is_dead_end(condition))
			{
				printf("	next state ");
				print_statename(stdout, condition);
				printf(" is dead-end\n");
			}
		}
	}

	// pc
	condition = changePC(state);
	if((state & 8) >> 3 == (state & 1))
	{
		if (condition != -1)
		{
			if (is_visited(visited, level, condition))
			{
				printf("	next state ");
				print_statename(stdout, condition);
				printf(" has been visited\n");
			}
			else
			{
				dfs_main(condition, goal_state, level + 1, visited);
				printf("back to ");
				print_statename(stdout, state);
				printf(" (level %d)\n", level);
			}
		}
		else
		{
			condition = state;	
			(state & 8) ? (condition &= ~8) : (condition |= 8); 
			(state & 1) ? (condition &= ~1) : (condition |= 1);
			if (is_dead_end(condition))
			{
				printf("	next state ");
				print_statename(stdout, condition);
				printf(" is dead-end\n");
			}
		}
	}
	//visited[level] = 0;
}

void make_adjacency_matrix( int graph[][16])
{
	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			if (!is_dead_end(i)) graph[i][j] = is_possible_transition(i, j);
}

void print_graph( int graph[][16], int num)
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j< 16; j++)
			printf("%d\t", graph[i][j]);
		printf("\n");
	}
}

void save_graph( char *filename, int graph[][16], int num)
{
	FILE* fp = fopen(filename, "w");
	fprintf(fp, "*Vertices 16\n");

	for (int i = 0; i < 16; i++)
	{
		fprintf(fp, "%d \"", i + 1);
		print_statename(fp, i);
		fprintf(fp, "\"\n");
	}

	fprintf(fp, "*Edges\n");
	for (int i = 0; i < num; i++)
		for (int j = i; j < num; j++)
			if (graph[i][j])
				fprintf(fp, "  %d  %d\n", i + 1, j + 1);

	fclose(fp);
}
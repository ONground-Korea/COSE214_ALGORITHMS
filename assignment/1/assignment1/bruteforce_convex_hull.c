#include <stdlib.h> // atoi, rand, malloc, realloc
#include <stdio.h>
#include <time.h> //time

#define RANGE 10000

typedef struct
{
	int x;
	int y;
} t_point;

typedef struct
{
	t_point from;
	t_point to;
} t_line;

////////////////////////////////////////////////////////////////////////////////
void print_header( char *filename)
{
	printf( "#! /usr/bin/env Rscript\n");
	printf( "png(\"%s\", width=700, height=700)\n", filename);
	
	printf( "plot(1:%d, 1:%d, type=\"n\")\n", RANGE, RANGE);
}
////////////////////////////////////////////////////////////////////////////////
void print_footer( void)
{
	printf( "dev.off()\n");
}

////////////////////////////////////////////////////////////////////////////////
/*
#points
points(2247,7459)
points(616,2904)
points(5976,6539)
points(1246,8191)
*/
void print_points( t_point *points, int num_point);

/*
#line segments
segments(7107,2909,7107,2909)
segments(43,8,5,38)
segments(43,8,329,2)
segments(5047,8014,5047,8014)
*/
void print_line_segments( t_line *lines, int num_line);

// [input] points : set of points
// [input] num_point : number of points
// [output] num_line : number of line segments that forms the convex hull
// return value : set of line segments that forms the convex hull
t_line *convex_hull( t_point *points, int num_point, int *num_line);

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char **argv)
{
	int x, y;
	int num_point; // number of points
	int num_line; // number of lines
	
	if (argc != 2)
	{
		printf( "%s number_of_points\n", argv[0]);
		return 0;
	}

	num_point = atoi( argv[1]);
	if (num_point <= 0)
	{
		printf( "The number of points should be a positive integer!\n");
		return 0;
	}

	t_point *points = (t_point *) malloc( num_point * sizeof( t_point));
		
	t_line *lines;

	// making n points
	srand( time(NULL));
	for (int i = 0; i < num_point; i++)
	{
		x = rand() % RANGE + 1; // 1 ~ RANGE random number
		y = rand() % RANGE + 1;
		
		points[i].x = x;
		points[i].y = y;
 	}

	fprintf( stderr, "%d points created!\n", num_point);

	print_header( "convex.png");
	
	print_points( points, num_point);
	
	lines = convex_hull( points, num_point, &num_line);

	fprintf( stderr, "%d lines created!\n", num_line);

	print_line_segments( lines, num_line);
		
	print_footer();
	
	free( points);
	free( lines);
	
	return 0;
}

void print_points( t_point *points, int num_point)
{
	printf("#points\n");
	for (int i = 0; i < num_point; i++)
	{
		printf("points(%d,%d)\n", points[i].x, points[i].y);
	}
}

void print_line_segments(t_line* lines, int num_line)
{
	printf("#line segments\n");
	for (int i = 0; i < num_line; i++)
	{
		printf("segments(%d,%d,%d,%d)\n", lines[i].from.x, lines[i].from.y, lines[i].to.x, lines[i].to.y);
	}
}

t_line* convex_hull(t_point* points, int num_point, int* num_line)
{
	int line_seg_size = 10;
	t_line* line_segment = (t_line*)malloc(sizeof(t_line) * line_seg_size); // 과제 영상과 동일하게 우선 10개 동적할당
	*num_line = 0; // num_line 개수 초기화

	for (int i = 0; i < num_point - 1; i++)
	{
		t_point point1 = points[i];

		for (int j = i + 1; j < num_point; j++)
		{
			t_point point2 = points[j]; // ax + by = c 직선 만들기 위함.
			int a = point2.y - point1.y; // a = y2 - y1
			int b = point1.x - point2.x; // b = x1 - x2
			int c = point1.x * point2.y - point1.y * point2.x; // c = x1y2 - y1x2

			int dir = -1; // 직선과 점 사이의 방향
			int isconvex = 1; // convex hull 판별. 우선 convex hull이라고 가정.

			for (int k = 0; k < num_point; k++) // 직선과 점들의 위치 관계 파악. 두 점의 순서에 따라 양수 음수 관계가 달라지므로 우선 한점과 파악.
			{
				if (a * points[k].x + b * points[k].y - c > 0)
				{
					dir = 1;
					break;
				}
				else if (a * points[k].x + b * points[k].y - c < 0)
				{
					dir = 0;
					break;
				}
			}

			// convex hull 판별
			if (dir == 1)
			{
				for (int k = 0; k < num_point; k++)
				{
					if (a * points[k].x + b * points[k].y - c < 0) // 방향이 다른 점 존재. convex hull이 아님.
					{
						isconvex = -1;
						break;
					}
				}
			}
			else if (dir == 0)
			{
				for (int k = 0; k < num_point; k++)
				{
					if (a * points[k].x + b * points[k].y - c > 0) // 방향이 다른 점 존재. convex hull이 아님.
					{
						isconvex = -1;
						break;
					}
				}
			}

			if (isconvex == 1)
			{
				(*num_line)++;
				if (*num_line >= 10)
				{
					line_seg_size += 10;
					line_segment = (t_line*)realloc(line_segment, sizeof(t_line) * line_seg_size); // 10개씩 추가로 할당.
				}

				int idx = (*num_line) - 1;
				line_segment[idx].from.x = point1.x;
				line_segment[idx].from.y = point1.y;
				line_segment[idx].to.x = point2.x;
				line_segment[idx].to.y = point2.y;
			}
		}
	}

	return line_segment;
}
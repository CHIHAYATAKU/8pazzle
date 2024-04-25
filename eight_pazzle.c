#include <stdio.h>
#include <stdlib.h>

#define N 3
#define Buf 9

typedef struct PUZZLESTATE
{
    int status[N][N];   // 盤面
    int space_row;      // 空マスの行
    int space_col;      // 空マスの列
    int predicted_cost; // 予測コスト
} PuzzleState;

void move(PuzzleState *state, int index, int direction, int updated_status[N][N]);
void space_locate(PuzzleState *state);
int heuristic1(int status[N][N], int goal[N][N]);
int heuristic2(int status[N][N], int goal[N][N]);
int heuristic3(int status[N][N], int goal[N][N]);
PuzzleState PuzzleStateInit(int status[N][N], int goal[N][N]);
int distance(const int *prev, int node);
int can_puzzle(const PuzzleState *state, int goal[N][N]);
int is_same(const PuzzleState *state1, int index, int state2[N][N]);
void add_state(PuzzleState *state, int *count_state, int *prev, int *closed, int **tree, int *arc_num, int top, int goal[N][N]);
int judge_finish(const PuzzleState *state, int index, int goal[N][N]);
void print_route(const PuzzleState *state, int prev[], int g);

int main(void)
{
    int **tree;
    int *open;
    int *closed;
    int *prev;
    int size = 0;
    int start[N][N] = {{1, 8, 0}, {4, 3, 2}, {5, 7, 6}};
    int goal[N][N] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 0}};
    int top, i, mindex, tmp, arc_num = 0, count_state = 0;
    int rows = 1, cols = 2;
    int paterns;
    int count_search = 0;
    PuzzleState *state;

    for (i = Buf; i > 0; i--)
    {
        rows = rows * i;
    }
    rows /= 2;
    paterns = rows;

    tree = (int **)malloc(rows * sizeof(int *));
    prev = (int *)malloc(paterns * sizeof(int));
    open = (int *)malloc(paterns * sizeof(int));
    closed = (int *)malloc(paterns * sizeof(int));
    state = (PuzzleState *)malloc(paterns * sizeof(PuzzleState));

    state[count_state++] = PuzzleStateInit(start, goal);

    if (can_puzzle(state, goal) != 0)
    {
        printf("The start can't match goal.\n");
        return 0;
    }

    for (i = 0; i < rows; i++)
    {
        tree[i] = (int *)malloc(cols * sizeof(int));
    }

    for (i = 0; i < paterns; i++)
    {
        prev[i] = -1;
        closed[i] = 0;
    }

    open[size++] = 0;
    closed[0] = 1;

    while (size > 0)
    {
        mindex = 0;

        for (i = 1; i < size; i++)
        {
            if (distance(prev, open[mindex]) + state[open[mindex]].predicted_cost > distance(prev, open[i]) + state[open[i]].predicted_cost)
            {
                mindex = i;
            }
        }

        tmp = open[size - 1];
        open[size - 1] = open[mindex];
        open[mindex] = tmp;

        top = open[--size];
        printf("%d,", top);
        count_search++;
        if (judge_finish(state, top, goal) == 0)
        {
            printf("\nOK\n");
            print_route(state, prev, top);

            printf("Number of search:%d\n", count_search);
            printf("Number of steps:%d\n", distance(prev, top));
            for (i = 0; i < rows; i++)
            {
                free(tree[i]);
            }
            free(tree);
            free(open);
            free(closed);
            free(prev);
            free(state);
            return 0;
        }

        add_state(state, &count_state, prev, closed, tree, &arc_num, top, goal);

        for (i = 0; i < arc_num; i++)
        {
            if (tree[i][0] == top)
            {
                if (closed[tree[i][1]] == 0)
                {
                    open[size++] = tree[i][1];
                    prev[tree[i][1]] = top;
                    closed[tree[i][1]] = 1;
                }
                else
                {
                    if ((distance(prev, top) + 1) < distance(prev, tree[i][1]))
                    {
                        prev[tree[i][1]] = top;
                    }
                }
            }
        }
    }
    printf("\nNG\n");
    for (i = 0; i < rows; i++)
    {
        free(tree[i]);
    }
    free(tree);
    free(open);
    free(closed);
    free(prev);
    free(state);
    return 0;
}

/* 移動後の配置 */
void move(PuzzleState *state, int index, int direction, int updated_status[N][N])
{
    int i, j, tmp;
    int err_flag = 0;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            updated_status[i][j] = state[index].status[i][j];
        }
    }

    tmp = updated_status[state[index].space_row][state[index].space_col];
    switch (direction)
    {
    case 0:
    {
        if ((state[index].space_row - 1) >= 0)
        {
            updated_status[state[index].space_row][state[index].space_col] = updated_status[state[index].space_row - 1][state[index].space_col];
            updated_status[state[index].space_row - 1][state[index].space_col] = tmp;
            break;
        }
        else
        {
            err_flag = 1;
            break;
        }
    }

    case 1:
    {
        if ((state[index].space_col + 1) < N)
        {
            updated_status[state[index].space_row][state[index].space_col] = updated_status[state[index].space_row][state[index].space_col + 1];
            updated_status[state[index].space_row][state[index].space_col + 1] = tmp;
            break;
        }
        else
        {
            err_flag = 1;
            break;
        }
    }

    case 2:
    {
        if ((state[index].space_row + 1) < N)
        {
            updated_status[state[index].space_row][state[index].space_col] = updated_status[state[index].space_row + 1][state[index].space_col];
            updated_status[state[index].space_row + 1][state[index].space_col] = tmp;
            break;
        }
        else
        {
            err_flag = 1;
            break;
        }
    }

    case 3:
    {
        if ((state[index].space_col - 1) >= 0)
        {
            updated_status[state[index].space_row][state[index].space_col] = updated_status[state[index].space_row][state[index].space_col - 1];
            updated_status[state[index].space_row][state[index].space_col - 1] = tmp;
            break;
        }
        else
        {
            err_flag = 1;
            break;
        }
    }

    default:
        err_flag = 1;
        break;
    }

    if (err_flag == 1)
    {
        updated_status[0][0] = -1;
    }

    return;
}

/* 空マスの位置を特定 */
void space_locate(PuzzleState *state)
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (state->status[i][j] == 0)
            {
                state->space_row = i;
                state->space_col = j;
                return;
            }
        }
    }
    return;
}

/* ヒューリスティック関数（各コマのゴールまでのマンハッタン距離の総和） */
int heuristic1(int status[N][N], int goal[N][N])
{
    int h = 0;
    int x_status, y_status, x_goal, y_goal;
    int flag = 0;
    for (x_status = 0; x_status < N; x_status++)
    {
        for (y_status = 0; y_status < N; y_status++)
        {
            for (x_goal = 0; x_goal < N; x_goal++)
            {
                for (y_goal = 0; y_goal < N; y_goal++)
                {
                    if (status[x_status][y_status] != 0 && status[x_status][y_status] == goal[x_goal][y_goal])
                    {
                        flag = 1;
                        break;
                    }
                }
                if (flag == 1)
                {
                    break;
                }
            }
            if (flag == 1)
            {
                h += (abs(x_goal - x_status) + abs(y_goal - y_status));
                flag = 0;
            }
        }
    }
    return h;
}

/* ヒューリスティック関数（各コマのゴールまでのマンハッタン距離の総和の２乗） */
int heuristic2(int status[N][N], int goal[N][N])
{
    int h = 0;
    int x_status, y_status, x_goal, y_goal;
    int flag = 0;
    for (x_status = 0; x_status < N; x_status++)
    {
        for (y_status = 0; y_status < N; y_status++)
        {
            for (x_goal = 0; x_goal < N; x_goal++)
            {
                for (y_goal = 0; y_goal < N; y_goal++)
                {
                    if (status[x_status][y_status] != 0 && status[x_status][y_status] == goal[x_goal][y_goal])
                    {
                        flag = 1;
                        break;
                    }
                }
                if (flag == 1)
                {
                    break;
                }
            }
            if (flag == 1)
            {
                h += (abs(x_goal - x_status) + abs(y_goal - y_status)) * (abs(x_goal - x_status) + abs(y_goal - y_status));
                flag = 0;
            }
        }
    }
    return h;
}

/* ヒューリスティック関数（各コマのゴールまでのマンハッタン距離の総和×2） */
int heuristic3(int status[N][N], int goal[N][N])
{
    int h = 0;
    int x_status, y_status, x_goal, y_goal;
    int flag = 0;
    for (x_status = 0; x_status < N; x_status++)
    {
        for (y_status = 0; y_status < N; y_status++)
        {
            for (x_goal = 0; x_goal < N; x_goal++)
            {
                for (y_goal = 0; y_goal < N; y_goal++)
                {
                    if (status[x_status][y_status] != 0 && status[x_status][y_status] == goal[x_goal][y_goal])
                    {
                        flag = 1;
                        break;
                    }
                }
                if (flag == 1)
                {
                    break;
                }
            }
            if (flag == 1)
            {
                h += (abs(x_goal - x_status) + abs(y_goal - y_status));
                flag = 0;
            }
        }
    }
    return h * 2;
}

/* Puzzle構造体変数を初期化する */
PuzzleState PuzzleStateInit(int status[N][N], int goal[N][N])
{
    int i, j;
    PuzzleState state = {
        {{0, 0, 0},
         {0, 0, 0},
         {0, 0, 0}},
        0,
        0,
        0};
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            state.status[i][j] = status[i][j];
        }
    }

    space_locate(&state);
    state.predicted_cost = heuristic1(status, goal);
    return state;
}

/* 深さ（nodeまでの手数） */
int distance(const int *prev, int node)
{
    int tmp = node, d = 0;

    while (prev[tmp] != -1)
    {
        d += 1;
        tmp = prev[tmp];
    }
    return d;
}

/* スタートがゴールまでたどり着ける配置か */
int can_puzzle(const PuzzleState *state, int goal[N][N])
{
    int i, j, k, l;
    int tmp;
    int count_swap = 0;
    PuzzleState state_tmp = state[0];
    PuzzleState goal_state = PuzzleStateInit(goal, goal);
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            for (k = 0; k < N; k++)
            {
                for (l = 0; l < N; l++)
                {
                    if (goal[i][j] == state_tmp.status[k][l] && (i != k || j != l))
                    {
                        tmp = state_tmp.status[k][l];
                        state_tmp.status[k][l] = state_tmp.status[i][j];
                        state_tmp.status[i][j] = tmp;
                        count_swap++;
                    }
                }
            }
        }
    }
    if (count_swap % 2 == (abs(state[0].space_col - goal_state.space_col) + abs(state[0].space_row - goal_state.space_row)) % 2)
    {
        return 0;
    }
    return -1;
}

/* 既にある盤面かを判定 */
int is_same(const PuzzleState *state1, int index, int state2[N][N])
{
    int i, j, k, flag;
    for (k = 0; k < index; k++)
    {
        flag = 0;
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (state1[k].status[i][j] != state2[i][j])
                {
                    flag = 1;
                }
            }
        }
        if (flag == 0)
        {
            return k;
        }
    }
    return -1;
}

/* 盤面を記録する構造体stateに追加（上右下左の順） */
void add_state(PuzzleState *state, int *count_state, int *prev, int *closed, int **tree, int *arc_num, int top, int goal[N][N])
{
    int direction;
    int updated_state[N][N];
    int return_issame;
    int count_tmp = *count_state;
    for (direction = 0; direction <= 3; direction++)
    {
        move(state, top, direction, updated_state);
        if (updated_state[0][0] != -1)
        {
            if ((return_issame = is_same(state, count_tmp, updated_state)) == -1)
            {
                state[(*count_state)++] = PuzzleStateInit(updated_state, goal);
                tree[*arc_num][0] = top;
                tree[*arc_num][1] = (*count_state) - 1;
                (*arc_num)++;
            }
            else
            {
                tree[*arc_num][0] = top;
                tree[*arc_num][1] = return_issame;
                (*arc_num)++;
            }
        }
    }
    return;
}

/* ゴールと同じか判定 */
int judge_finish(const PuzzleState *state, int index, int goal[N][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (state[index].status[i][j] != goal[i][j])
            {
                return -1;
            }
        }
    }
    return 0;
}

/* 結果のルートを表示 */
void print_route(const PuzzleState *state, int prev[], int g)
{
    int tmp;
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            printf("%d ", state[g].status[i][j]);
        }
        printf("\n");
    }

    tmp = prev[g];
    while (tmp >= 0)
    {
        printf(" <-\n");
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                printf("%d ", state[tmp].status[i][j]);
            }
            printf("\n");
        }
        tmp = prev[tmp];
    }
    printf("\n");
    return;
}
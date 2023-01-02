/*
 * ��� ���������� ���������� �������� ���� -lncurses
 * gcc -o snake main.c -lncurses

 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <inttypes.h>
#include <wchar.h>

enum {
    LEFT = 1, UP = 2, RIGHT = 3, DOWN = 4, STOP_GAME = 'q'
};
enum {
    MAX_TAIL_SIZE = 1000,
    START_TAIL_SIZE = 3,
    MAX_FOOD_SIZE = 20,
    FOOD_EXPIRE_SECONDS = 10,
    SPEED = 20000,
    SEED_NUMBER = 3
};

enum{
    SNAKE1 = 1,
    SNAKE2 = 2,
    FOOD = 3,
    SNAKE3 = 4
};

/*
 ����� ���� ������ ��������� �� ��������� x,y
 */
struct tail {
    int x;
    int y;
} tail[MAX_TAIL_SIZE], tail2[MAX_TAIL_SIZE], tail3[MAX_TAIL_SIZE];

/*
 ��� ������ �����
 x, y - ���������� ��� ����������� �����
 put_time - ����� ����� ������ ����� ���� �����������
 point - ������� ��� ����� ('$','E'...)
 enable - ���� �� ����� �������
 */
struct food {
    int x;
    int y;
    time_t put_time;
    char point;
    uint8_t enable;
} food[MAX_FOOD_SIZE];

/*
 ������ ������ �������� � ����
 x,y - ���������� ������� �������
 direction - ����������� ��������
 tsize - ������ ������
 *tail -  ������ �� �����
 */
struct snake {
    int number;
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail *tail;
} snake, snake2, snake3;

void setColor(int objectType){
    attroff(COLOR_PAIR(1));
    attroff(COLOR_PAIR(2));
    attroff(COLOR_PAIR(3));
    attroff(COLOR_PAIR(4));
    switch (objectType){
        case 1:{ // SNAKE1
            attron(COLOR_PAIR(1));
            break;
        }
        case 2:{ // SNAKE2
            attron(COLOR_PAIR(2));
            break;
        }
        case 3:{ // FOOD
            attron(COLOR_PAIR(3));
            break;
        }

        case 4:{ // SNAKE3
            attron(COLOR_PAIR(4));
            break;
        }
    }
}

/*
 �������� ������ � ������ �������� ����������� ��������
 */
void go(struct snake *head) {
    setColor(head->number);
    char ch[] = "@";
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x); // macro - ������ ���������
    //clear(); // ������� ���� �����
    mvprintw(head->y, head->x, " "); // ������� ���� ������
    switch (head->direction) {
        case LEFT:
            if (head->x <= 0) // ����������� ��������, ��� �� ��
                // ������� �� ������� ������
                head->x = max_x;
            mvprintw(head->y, --(head->x), ch);
            break;
        case RIGHT:
            if (head->x >= max_x)
                head->x = 0;
            mvprintw(head->y, ++(head->x), ch);
            break;
        case UP:
            if (head->y <= 0)
                head->y = max_y;
            mvprintw(--(head->y), head->x, ch);
            break;
        case DOWN:
            if (head->y >= max_y)
                head->y = 0;
            mvprintw(++(head->y), head->x, ch);
            break;
        default:
            break;
    }
    refresh();
}

void changeDirection(int32_t *new_direction, int32_t key) {
    switch (key) {
        case KEY_DOWN: // ������� ����
            *new_direction = DOWN;
            break;
        case KEY_UP: // ������� �����
            *new_direction = UP;
            break;
        case KEY_LEFT: // ������� �����
            *new_direction = LEFT;
            break;
        case KEY_RIGHT: // ������� ������
            *new_direction = RIGHT;
            break;
        default:
            break;
    }
}


int distance(const struct snake snake, const struct food food) {   // ��������� ���������� ����� �� ���
    return (abs(snake.x - food.x) + abs(snake.y - food.y));
}

void autoChangeDirection(struct snake *snake, struct food food[], int foodSize) {
    int pointer = 0;
    for (int i = 1; i < foodSize; i++) {   // ���� ��������� ���
        pointer = (distance(*snake, food[i]) < distance(*snake, food[pointer])) ? i : pointer;
    }
    if ((snake->direction == RIGHT || snake->direction == LEFT) &&
        (snake->y != food[pointer].y)) {  // �������������� ��������
        snake->direction = (food[pointer].y > snake->y) ? DOWN : UP;
    } else if ((snake->direction == DOWN || snake->direction == UP) &&
               (snake->x != food[pointer].x)) {  // ������������ ��������
        snake->direction = (food[pointer].x > snake->x) ? RIGHT : LEFT;
    }
}


int checkDirection(int32_t dir, int32_t key) {
    if (KEY_DOWN == key && dir == UP || KEY_UP == key && dir == DOWN || KEY_LEFT == key && dir == RIGHT ||
        KEY_RIGHT == key && dir == LEFT) {
        return 0;
    } else {
        return 1;
    }
}

void initTail(struct tail t[], size_t size) {
    struct tail init_t = {0, 0};
    for (size_t i = 0; i < size; i++) {
        t[i] = init_t;
    }
}

void initHead(struct snake *head) {
    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);
    head->x = (rand()) % (max_x - 1);
    head->y = (rand()) % (max_y - 10) + 1;
    head->direction = RIGHT;
}

void initFood(struct food f[], size_t size) {
    struct food init = {0, 0, 0, 0, 0};
    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);
    for (size_t i = 0; i < size; i++) {
        f[i] = init;
    }
}

void init(struct snake *head, int number, struct tail *tail, size_t size) {
    clear(); // ������� ���� �����
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head);
    head->number = number;
    head->tail = tail; // ����������� � ������ �����
    head->tsize = size + 1;
}

/*
 �������� ������ � ������ �������� ������
 */
void goTail(struct snake *head) {
    char ch[] = "*";
    setColor(head->number);
    mvprintw(head->tail[head->tsize - 1].y, head->tail[head->tsize - 1].x, " ");
    for (size_t i = head->tsize - 1; i > 0; i--) {
        head->tail[i] = head->tail[i - 1];
        if (head->tail[i].y || head->tail[i].x)

            mvprintw(head->tail[i].y, head->tail[i].x, ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

/*
 ���������� ������ �� 1 �������
 */
void addTail(struct snake *head) {
    if (head == NULL || head->tsize > MAX_TAIL_SIZE) {
        mvprintw(0, 0, "Can't add tail");
        return;
    }
    head->tsize++;
}

void printHelp(char *s) {
    mvprintw(0, 0, s);
}

/*
 ��������/���������� ������� ����� �� ����
 */
void putFoodSeed(struct food *fp) {
    int max_x = 0, max_y = 0;
    char spoint[2] = {0};
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp->y, fp->x, " ");
    fp->x = rand() % (max_x - 1);
    fp->y = rand() % (max_y - 2) + 1; //�� �������� ������� ������
    fp->put_time = time(NULL);
    fp->point = '$';
    fp->enable = 1;
    spoint[0] = fp->point;
    setColor(FOOD);
    mvprintw(fp->y, fp->x, spoint);
}

// ������ ������, ����� ��� ��� ��� ��������
void blinkFood(struct food fp[], size_t nfood) {
    time_t current_time = time(NULL);
    char spoint[2] = {0}; // ��� �������� ����� '$','\0'
    for (size_t i = 0; i < nfood; i++) {
        if (fp[i].enable && (current_time - fp[i].put_time) > 6) {
            spoint[0] = (current_time % 2) ? 'S' : 's';
            setColor(FOOD);
            mvprintw(fp[i].y, fp[i].x, spoint);
        }
    }
}

void repairSeed(struct food f[], size_t nfood, struct snake *head) {
    for (size_t i = 0; i < head->tsize; i++)
        for (size_t j = 0; j < nfood; j++) {
            /* ���� ����� ��������� � ������ */
            if (f[j].x == head->tail[i].x && f[j].y == head->tail[i].y && f[i].enable) {
                mvprintw(0, 0, "Repair tail seed %d", j);
                putFoodSeed(&f[j]);
            }
        }
    for (size_t i = 0; i < nfood; i++)
        for (size_t j = 0; j < nfood; j++) {
            /* ���� ��� ����� �� ����� ����� */
            if (i != j && f[i].enable && f[j].enable && f[j].x == f[i].x && f[j].y == f[i].y && f[i].enable) {
                mvprintw(0, 0, "Repair same seed %d", j);
                putFoodSeed(&f[j]);
            }
        }
}

/*
 ���������� ��� �� ����
 */
void putFood(struct food f[], size_t number_seeds) {
    for (size_t i = 0; i < number_seeds; i++) {
        putFoodSeed(&f[i]);
    }
}

void refreshFood(struct food f[], int nfood) {
    int max_x = 0, max_y = 0;
    char spoint[2] = {0};
    getmaxyx(stdscr, max_y, max_x);
    for (size_t i = 0; i < nfood; i++) {
        if (f[i].put_time) {
            if (!f[i].enable || (time(NULL) - f[i].put_time) > FOOD_EXPIRE_SECONDS) {
                putFoodSeed(&f[i]);
            }
        }
    }
}

_Bool haveEat(struct snake *head, struct food f[]) {
    for (size_t i = 0; i < MAX_FOOD_SIZE; i++)
        if (f[i].enable && head->x == f[i].x && head->y == f[i].y) {
            f[i].enable = 0;
            return 1;
        }
    return 0;
}

void printLevel(struct snake *head) {
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    if (head->number == SNAKE1){
        setColor(head->number);
        mvprintw(0, max_x - 10, "LEVEL: %d", head->tsize);
    }
    if (head->number == SNAKE2){
        setColor(head->number);
        mvprintw(1, max_x - 10, "LEVEL: %d", head->tsize);
    }
}

void printExit(struct snake *head) {
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(max_y / 2, max_x / 2 - 5, "Your LEVEL is %d", head->tsize);
}

_Bool isCrash(struct snake *head) {
    for (size_t i = 1; i < head->tsize; i++)
        if (head->x == head->tail[i].x && head->y == head->tail[i].y)
            return 1;
    return 0;
}

int main() {
    char ch[] = "*";
    int x = 0, y = 0, key_pressed = 0;
    init(&snake, 1, tail, START_TAIL_SIZE); //�������������, ����� = 3
    init(&snake2, 2, tail2, START_TAIL_SIZE); //�������������, ����� = 3
    init(&snake3, 4, tail3, START_TAIL_SIZE*10); //�������������, ����� = 30

    initFood(food, MAX_FOOD_SIZE);
    initscr();            // ����� curses mod
    keypad(stdscr, TRUE); // �������� F1, F2, ������� � �.�.
    raw();                // ��������� line buffering
    noecho();            // ��������� echo() ����� ��� ������ getch
    curs_set(FALSE);    //��������� ������
    printHelp("  Use arrows for control. Press 'q' for EXIT");
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    putFood(food, SEED_NUMBER);// ������ �����
    timeout(0);    //��������� ������� ����� ������� ������� � �����
    while (key_pressed != STOP_GAME) {
        key_pressed = getch(); // ��������� �������
        if (checkDirection(snake.direction, key_pressed)) //�������� ������������ ����� �����������
        {
            changeDirection(&snake.direction, key_pressed); // ������ ����������� ��������
        }
        autoChangeDirection(&snake2, food, SEED_NUMBER);
        autoChangeDirection(&snake3, food, 1);
        if (isCrash(&snake))
            break;
        go(&snake); // ������ ����� ������
        goTail(&snake); //������ �����
        go(&snake2); // ������ ����� ������
        goTail(&snake2); //������ �����

        go(&snake3); // ������ ����� ������
        goTail(&snake3); //������ �����

        if (haveEat(&snake, food)) {
            addTail(&snake);
            printLevel(&snake);
        }
        if (haveEat(&snake2, food)) {
            addTail(&snake2);
            printLevel(&snake2);
        }
        refreshFood(food, SEED_NUMBER);// ��������� ���
        repairSeed(food, SEED_NUMBER, &snake);
        blinkFood(food, SEED_NUMBER);
        timeout(100); // �������� ��� ���������
    }
    setColor(SNAKE1);
    printExit(&snake);
    timeout(SPEED);
    getch();
    endwin(); // ��������� ����� curses mod

    return 0;
}

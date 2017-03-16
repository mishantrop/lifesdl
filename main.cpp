#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <iostream>

using namespace std;

unsigned short field[18][18]; // 16x16 + запасные
unsigned short field2[18][18]; // 16x16 + запасные
unsigned short sum = 0; // Сумма клеток вокруг клетки
bool paused = false; // Состояние процесса по умолчанию
bool step = true; // Пошаговое выполнение программы
unsigned short stat[160]; // массив для отображения статистики

struct obj
{
    int x;
    int y;
    unsigned short i;
    unsigned short n;
} mouse;

unsigned short init(); // Инициализация SDL
unsigned short init_field(); // Расставить клетки по клеткам :)
unsigned short field_clear(); // Очистить поле
unsigned short count(); // Подсчёт живых клеток
unsigned short shift_stat(); // Сдвинуть статистику влево
unsigned short show_stat(); // Нарисовать линии статистики

int main ()
{
    init();
    init_field();

    const char *datetime = "file";
    const char *format = ".BMP";

    SDL_Surface* screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if (!screen ) {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

/*
    SDL_version compiled;
    SDL_VERSION(&compiled);
    printf("SDL version: %d.%d.%d \n", compiled.major, compiled.minor, compiled.patch);
*/
    SDL_Surface* bacg = SDL_LoadBMP("bg.bmp");
    SDL_Surface* sone = SDL_LoadBMP("1.bmp");
    SDL_Surface* szer = SDL_LoadBMP("0.bmp");

    // Controls
    SDL_Surface* sbplay     = SDL_LoadBMP("b_play.bmp");
    SDL_Surface* sbpause    = SDL_LoadBMP("b_pause.bmp");
    SDL_Surface* sbclean    = SDL_LoadBMP("b_clean.bmp");
    SDL_Surface* sbshuffle  = SDL_LoadBMP("b_shuffle.bmp");
    SDL_Surface* sbstep     = SDL_LoadBMP("b_step.bmp");
    SDL_Surface* sbstat     = SDL_LoadBMP("b_stat.bmp");
    SDL_Surface* spixel     = SDL_LoadBMP("pixel.bmp");

    SDL_Rect bag, rbplay, rbpause, rbclean, rbshuffle, rbstep, rone, rzer, rbstat, rpixel;

    bag.x       = 0;   bag.y       = 0;
    rbplay.x    = 432; rbplay.y    = 24;
    rbpause.x   = 432; rbpause.y   = 24;
    rbstep.x    = 544; rbstep.y    = 24;
    rbclean.x   = 432; rbclean.y   = 168;
    rbshuffle.x = 432; rbshuffle.y = 96;
    rbstat.x    = 432; rbstat.y    = 240;
    rone.x      = 0;   rone.y      = 0;
    rzer.x      = 0;   rzer.y      = 0;
    rpixel.x    = 0;   rpixel.y    = 0;

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_KEYDOWN:
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        done = true;
                    } else if (event.key.keysym.sym == SDLK_c) {
                        // Очистить поле
                        field_clear();
                    } else if (event.key.keysym.sym == SDLK_m) {
                        // Заново перемешать
                        init_field();
                    } else if (event.key.keysym.sym == SDLK_t) {
                        // Продолжить выполнение программы
                            step = !step;
                            paused = !paused;
                       //paused = (step == true) ? !paused : paused;
                    } else if (event.key.keysym.sym == SDLK_s) {
                        // Пошаговое выполнение программы
                        step = (paused == true) ? !step : step;
                    } else if (event.key.keysym.sym == SDLK_f) {
                        // Скриншот
                        time_t rawtime;
                        time ( &rawtime );
                        datetime = ctime(&rawtime);
                        strcat((char*)datetime, (char*)format);
                        SDL_SaveBMP(screen, datetime);
                    }
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
                {
                    mouse.x = event.button.x;        // координаты курсора
                    mouse.y = event.button.y;        // координаты курсора
                    mouse.i = event.button.x/rone.w; // выбранный индекс поля
                    mouse.n = event.button.y/rone.h; // выбранный индекс поля
                    switch(event.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                        {
                            if (((mouse.x >= rbplay.x) && (mouse.x <= rbplay.x+48)) && ((mouse.y >= rbplay.y) && (mouse.y <= rbplay.y+48)))
                            {
                                // стоп/продолжить
                                paused = !paused;
                                step = !step;
                            }
                            else if (((mouse.x >= rbstep.x) && (mouse.x <= rbstep.x+48)) && ((mouse.y >= rbstep.y) && (mouse.y <= rbstep.y+48)))
                            {
                                // Пошаговое
                                step = (paused == true) ? !step : step;
                            }
                            else if (((mouse.x >= rbshuffle.x) && (mouse.x <= rbshuffle.x+160)) && ((mouse.y >= rbshuffle.y) && (mouse.y <= rbshuffle.y+48)))
                            {
                                // Перемешать
                                init_field();
                            }
                            else if (((mouse.x >= rbclean.x) && (mouse.x <= rbclean.x+160)) && ((mouse.y >= rbclean.y) && (mouse.y <= rbclean.y+48)))
                            {
                                // Очистить
                                field_clear();
                            }


                            field[mouse.i][mouse.n] = (((mouse.i < 17) && (mouse.i > 0)) && ((mouse.n < 17) && (mouse.n > 0))) ? 1 : field[mouse.i][mouse.n];
                            break;
                        }

                        case SDL_BUTTON_RIGHT:
                        {
                            field[mouse.i][mouse.n] = (((mouse.i < 17) && (mouse.i > 0)) && ((mouse.n < 17) && (mouse.n > 0))) ? 0 : field[mouse.i][mouse.n];
                            break;
                        }
                    }
                }
            }
        }

        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 20, 20, 20));
        SDL_BlitSurface(bacg, 0, screen, &bag); // background

        // Вычисление следующего поколения
        if (step == true) {
            if (paused == true) {
                shift_stat();
            }
            for (int i = 1; i < 17; i++)
            {
                for (int n = 1; n < 17; n++) {
                    sum = field[i-1][n-1] + field[i][n-1] + field[i+1][n-1] + field[i-1][n] + field[i+1][n] + field[i-1][n+1] + field[i][n+1] + field[i+1][n+1];
                    if (field[i][n] == 0) {
                        field2[i][n] = (sum == 3) ? 1 : field2[i][n];
                    } else if (field[i][n] == 1) {
                        field2[i][n] = ((sum == 3) || (sum == 2)) ? field[i][n] : 0;
                    }
                }
            }
            // Из временного поля в постоянное
            for (int i = 1; i < 17; i++)
            {
                for (int n = 1; n < 17; n++) {
                    field[i][n] = field2[i][n];
                    field2[i][n] = 0;
                }
            }
            step = (paused == true) ? false : true;
        }

        // Прорисовка
        for (int i = 1; i < 17; i++)
        {
            for (int n = 1; n < 17; n++) {
                rone.x = i*rone.w;
                rone.y = n*rone.h;
                rzer.x = i*rzer.w;
                rzer.y = n*rzer.h;
                if (field[i][n] == 1) {
                    SDL_BlitSurface(sone, 0, screen, &rone);
                } else {
                    SDL_BlitSurface(szer, 0, screen, &rzer);
                }
            }
        }

        // Draw buttons
        if (!paused) {
            SDL_BlitSurface(sbpause, 0, screen, &rbpause);
            shift_stat();
        } else {
            SDL_BlitSurface(sbplay, 0, screen, &rbplay);
            if (step == true) {
                break;
            }
        }
        SDL_BlitSurface(sbstep, 0, screen, &rbstep);
        SDL_BlitSurface(sbshuffle, 0, screen, &rbshuffle);
        SDL_BlitSurface(sbclean, 0, screen, &rbclean);
        SDL_BlitSurface(sbstat, 0, screen, &rbstat);
        count();


        // Прорисовка статистики
        for (unsigned short i = 157; i > 1; i--)
        {
            unsigned short value = stat[i]/6; // кол-во точек для рисования
            rpixel.x = 432+i;
            rpixel.y = 284-value+2;

            for (unsigned short c = 0; c < value; c++)
            {
                rpixel.y = rpixel.y+c;
                SDL_BlitSurface(spixel, 0, screen, &rpixel);
                rpixel.y = rpixel.y-c;
            }
        }

        //printf("Button %i at (%i, %i)\n", event.button.button, event.button.x, event.button.y);
        SDL_Flip(screen);
        usleep(40000);
    } // end main loop

    return 0;
}


unsigned short init()
{
    mouse.x = 0;
    mouse.y = 0;
    mouse.i = 0;
    mouse.n = 0;
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    } else {
        return 0;
    }
    atexit(SDL_Quit);
}

unsigned short field_clear()
{
    for (int i = 1; i < 17; i++)
    {
        for (int n = 1; n < 17; n++) {
            field[i][n] = 0;
            field2[i][n] = 0;
        }
    }
    return 0;
}

unsigned short init_field()
{
    for (int i = 1; i < 17; i++)
    {
        for (int n = 1; n < 17; n++) {
            field[i][n] = (rand()%2 == 1) ? 1 : 0;
            field2[i][n] = 0;
        }
    }
/*
    Планер
    field[8+1][8] = 1;    field[8+2][8+1] = 1;    field[8+2][8+2] = 1;    field[8+1][8+2] = 1;    field[8][8+2] = 1;
*/
    return 0;
}

unsigned short count()
{
    unsigned short count = 0;
    // посчитать кол-во клеток
    for (int i = 1; i < 17; i++)
    {
        for (int n = 1; n < 17; n++) {
            count += (field[i][n] == 1) ? 1 : 0;
        }
    }
    //printf("%i \n", count);
    return count;
}

unsigned short shift_stat()
{
    for (unsigned short i = 0; i < 159; i++)
    {
        stat[i] = stat[i+1];
    }
    stat[159] = count();
    return 0;
}

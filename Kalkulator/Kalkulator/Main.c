#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h> // Ikke i bruk, ble lagt til ved starten av for å unngå bryet med å gjøre det senere
#include "constants.c"

// Trenger å legge til bedre utregning og må vise ihvertfall de to første desimal tallene når de ikke bare er 0

SDL_Window* vindu = NULL;
SDL_Renderer* renderer = NULL;

int loop = FALSE;

int decimal_num = 0;
int save;
int decimals;

float display = 0;

int lengde = 0;

char contextual;

float* nummerene;
int* operasjoner;

int fjern = 0;

int knapp_egenskaper[][5] = { {10, 200, 70, 80 }, { -1, -1, -1, -1 }, { -1, -1, -1, -1 }, { 10, -2, -1, -1 },
    { -1, -1, -1, -1 }, { -1, -1, -1, -1 }, { 10, -2, -1, -1 }, { -1, -1, -1, -1 }, { -1, -1, -1, -1 }, {96, -2, -1, -1},
    { 300, 200, -1, -1 }, { -1, -1, -1, -1 }, { 300, -2, -1, -1 }, { -1, -1, -1, -1 }, { 300, -2, -1, -1 }, { -1, -1, -1, -1 }};

char knapp_tekst[][10] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "+", "-", "*", ":", ",", "="};

SDL_Surface* knapp_surfaces[sizeof(knapp_egenskaper) / 20];
SDL_Texture* knapp_textures[sizeof(knapp_egenskaper) / 20];

TTF_Font* standard_font;
SDL_Color tekst_farge = {0, 0, 0};

SDL_Surface* display_surface;
SDL_Texture* display_texture;

int last_frame_time = 0;

void update_display(char mode)
{
    if (mode == 'n') // normal
    {
        char holder[32];
        char display_str[100];
        char format_str[10];

        
        sprintf_s(holder, sizeof(holder), "%f", display);

        int størrelse = strlen(holder);

        int desimaler = -1;

        for (int i = 0; i < størrelse; i++)
        {
            if (holder[i] == '.')
            {
                desimaler = 0;
            }

            if (desimaler > -1 && holder[i] != '0')
            {
                desimaler += 1;
            }
        }
        desimaler--;


        sprintf_s(format_str, sizeof(format_str), "%%.%df", desimaler);
        sprintf_s(display_str, sizeof(display_str), format_str, display);
        display_surface = TTF_RenderText_Solid(standard_font, display_str, tekst_farge);

        if (!display_surface)
        {
            fprintf(stderr, "Det oppsto en error ved surface initialisering");
        }

        display_texture = SDL_CreateTextureFromSurface(renderer, display_surface);
    }
    else if (mode == 'o') // operand
    {
        char format[2];
        format[0] = contextual;
        format[1] = '\0';
        display_surface = TTF_RenderText_Solid(standard_font, format, tekst_farge);

        if (!display_surface)
        {
            fprintf(stderr, "Det oppsto en error ved surface initialisering");
        }

        display_texture = SDL_CreateTextureFromSurface(renderer, display_surface);
    }
}

void nummer(int num) {
    if (fjern == 1) display = 0;
    if (decimals == 0) decimal_num = 0;
    if (display != 0 && display < 1000000000 && decimals == 0)
    {
        display *= 10;
        display += num;
    } else if (display < 1000000000 && decimals == 0)
    {
        display = (float)num;
    }
    else if (display < 1000000000 && decimals == 1) {

        float factor = 10.0;

        for (int i = 0; i < decimal_num; i++)
        {
            factor *= 10.0;
        }
        display += ((float)num / factor);
        decimal_num += 1;
        update_display('n');
    }
    update_display('n');

    fjern = 0;
}

void operatør(char operator)
{
    if (lengde == 0 && display != 0)
    {
        lengde += 1;
        decimals = 0;
        save = decimal_num;
        decimal_num = 0;

        nummerene = (float*)malloc(lengde * sizeof(float));
        operasjoner = (int*)malloc(lengde * sizeof(float));

        if (nummerene == NULL || operasjoner == NULL)
        {
            fprintf(stderr, "Det oppsto en error ved allokering av minne til liste");
        }

        nummerene[lengde - 1] = display;
        
        switch (operator) 
        {
        case '+':
            operasjoner[lengde - 1] = 1;
            contextual = '+';
            break;
        case '-':
            operasjoner[lengde - 1] = 2;
            contextual = '-';
            break;
        case '*':
            operasjoner[lengde - 1] = 3;
            contextual = '*';
            break;
        case ':':
            operasjoner[lengde - 1] = 4;
            contextual = '/';
            break;
        }

        display = 0;
        update_display('o');
    } else if (display != 0)
    {
        lengde += 1;
        decimals = 0;
        save = decimal_num;
        decimal_num = 0;

        float* temp1 = (float*)realloc(nummerene, lengde * sizeof(float));
        int* temp2 = (int*)realloc(operasjoner, lengde * sizeof(int));

        if (temp1 == NULL || temp2 == NULL)
        {
            free(nummerene);
            free(operasjoner);
            fprintf(stderr, "Det oppsto en error ved allokering av minne til liste");
        } else
        {
            nummerene = temp1;
            operasjoner = temp2;
        }

        nummerene[lengde - 1] = display;
        
        switch (operator)
        {
        case '+':
            operasjoner[lengde - 1] = 1;
            contextual = '+';
            break;
        case '-':
            operasjoner[lengde - 1] = 2;
            contextual = '-';
            break;
        case '*':
            operasjoner[lengde - 1] = 3;
            contextual = '*';
            break;
        case ':':
            operasjoner[lengde - 1] = 4;
            contextual = '/';
            break;
        }

        display = 0;
        update_display('o');
    }
}

void lik()
{
    if (lengde >0)
    {
        float* temp = (float*)realloc(nummerene, (lengde +1) * sizeof(float));

        if (temp == NULL)
        {
            printf("skjedde noe?\n");
            free(nummerene);
            free(operasjoner);
            fprintf(stderr, "Det oppsto en error ved allokering av minne til liste");
        }

        nummerene = temp;
        nummerene[lengde] = display;

        float final = 0.0;

        float* nyenummerene = (float*)malloc(sizeof(float));;
        int* nyeoperasjoner = (int*)malloc(sizeof(int));;

        int nyelengde = 0;

        int ign = 0;

        for (int i = 0; i < lengde +1; i++)
        {
            float* temp1 = (float *)realloc(nyenummerene, (nyelengde + 1) * sizeof(float));
            int* temp2 = (int *)realloc(nyeoperasjoner, (nyelengde + 1) * sizeof(int));

            nyenummerene = temp1;
            nyeoperasjoner = temp2;

            if (operasjoner[i] == 3)
            {
                nyenummerene[i] = nummerene[i] * nummerene[i + 1];
                nyelengde += 1;
                ign = 1;
            } else if (operasjoner[i] == 4)
            {
                ign = 1;
                nyenummerene[i] = nummerene[i] / nummerene[i + 1];
                nyelengde += 1;
            } else if (ign == 0)
            {
                nyenummerene[i] = nummerene[i];
                if (i != 1 && operasjoner[i] != NULL) nyeoperasjoner[i] = operasjoner[i];
                nyelengde += 1;
            } else
            {
                ign = 0;
            }
        }

        free(nummerene);
        free(operasjoner);

        nummerene = (float*)malloc((nyelengde + 1) * sizeof(float));
        operasjoner = (int*)malloc((nyelengde + 1) * sizeof(int));

        lengde = nyelengde;

        for (int i = 0; i < lengde; i++)
        {
            nummerene[i] = nyenummerene[i];
            operasjoner[i] = nyeoperasjoner[i];
        }


        free(nyenummerene);
        free(nyeoperasjoner);


        display = nummerene[0];

        for (int i = 0; i < lengde; i++)
        {
            if (operasjoner[i] == 1)
            {
                if (i != 0)
                {
                    final += nummerene[i + 1];
                } else
                {
                    final += nummerene[i] + nummerene[i + 1];
                }
            } else if (operasjoner[i] == 2)
            {
                if (i != 0)
                {
                    final -= nummerene[i + 1];
                }
                else
                {
                    final = nummerene[i] - nummerene[i + 1];
                }
            } else if (operasjoner[i] == 3)
            {
                if (i != 0)
                {
                    final *= nummerene[i + 1];
                }
                else
                {
                    final = nummerene[i] * nummerene[i + 1];
                }
            } else if (operasjoner[i] == 4)
            {
                if (i != 0)
                {
                    final /= nummerene[i + 1];
                }
                else
                {
                    final = nummerene[i] / nummerene[i + 1];
                }
            }
        }

        if (final != 0) display = final;
        decimal_num = save;
        update_display('n');

        free(operasjoner);
        free(nummerene);
        lengde = 0;
        fjern = 1;
        decimals = 0;
    }
}


int initialiser_vindu()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Det oppsto en error ved SDL initialisering \n");
        return FALSE;
    }

    vindu = SDL_CreateWindow("Kalkulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, 
            WINDOWWIDTH,
            WINDOWHEIGHT,
            0);

    if (!vindu)
    {
        fprintf(stderr, "Det oppsto en error ved vindu initialisering \n");
        return FALSE;
    }

    renderer = SDL_CreateRenderer(vindu, -1, 0);

    if (!renderer)
    {
        fprintf(stderr, "Det oppsto en error ved renderer initialisering \n");
        return FALSE;
    }

    return TRUE;
}

void setup(){
    if (TTF_Init() != 0)
    {
        fprintf(stderr, "Det oppsto en error ved SDL_TTF initialisering \n");
    }

    standard_font = TTF_OpenFont("fonts/Standard_Font.ttf", 50);

    if (!standard_font)
    {
        fprintf(stderr, "Det oppsto en error ved font initialisering \n");
    }
    
    update_display('n');

    for (int i = 0; i < sizeof(knapp_egenskaper) / 20; i++)
    {
        knapp_surfaces[i] = TTF_RenderText_Solid(standard_font, knapp_tekst[i], tekst_farge);
        knapp_textures[i] = SDL_CreateTextureFromSurface(renderer, knapp_surfaces[i]);
    }
}

void inndata_prosessering()
{
    //10 200 70 80
    SDL_Event inndata;
    SDL_PollEvent(&inndata);

    int mouseX, mouseY;

    SDL_GetMouseState(&mouseX, &mouseY);

    if (inndata.type == SDL_MOUSEBUTTONDOWN) {
        int funnet = 0;
        int iterate = 0;

        while (funnet == 0 && iterate < sizeof(knapp_egenskaper)/20) {
            // Går litt langt, men det funker
            if (mouseX >knapp_egenskaper[iterate][0] && mouseX < knapp_egenskaper[iterate][0]+knapp_egenskaper[iterate][2] && mouseY > knapp_egenskaper[iterate][1] && mouseY < knapp_egenskaper[iterate][1]+knapp_egenskaper[iterate][3])
            {
                if (iterate < 9) {
                    nummer(iterate + 1);
                }

                if (iterate == 9)
                {
                    nummer(0);
                }

                if (iterate == 10)
                {
                    operatør('+');
                }

                if (iterate == 11)
                {
                    operatør('-');
                }

                if (iterate == 12)
                {
                    operatør('*');
                }

                if (iterate == 13)
                {
                    operatør(':');
                }

                if (iterate == 14)
                {
                    decimals = 1;
                    update_display('n');
                }

                if (iterate == 15)
                {
                    lik();
                }
            }
            iterate++;
        }
    }

    switch (inndata.type)
    {
        case SDL_QUIT:
            loop = FALSE;
            break;
    }
}

void oppdater()
{
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    last_frame_time = SDL_GetTicks();
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderClear(renderer);

    if (!display_texture)
    {
        fprintf(stderr, "Det oppsto en error ved tekst initialisering %s \n", SDL_GetError());
    } else 
    {
        SDL_Rect destinasjon = {20, 50, display_surface->w, display_surface->h}; // Posisjon til display   
        SDL_RenderCopy(renderer, display_texture, NULL, &destinasjon);
    }

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    int last[4] = {0, 0, 0, 0};


    for (int i = 0; i < sizeof(knapp_egenskaper) / 20; i++)
    {
        int egenskaper[5];

        for (int i2 = 0; i2 < 4; i2++)
        {
            if (knapp_egenskaper[i][i2] != -1 && knapp_egenskaper[i][i2] != -2)
            {
                egenskaper[i2] = knapp_egenskaper[i][i2];
                last[i2] = egenskaper[i2];
            } else if (knapp_egenskaper[i][i2] == -1)
            {
                egenskaper[i2] = last[i2];
                if (i2 == 0)
                {
                    egenskaper[0] += last[2] + 15;
                    last[0] = egenskaper[0];
                }
                knapp_egenskaper[i][i2] = egenskaper[i2];
            } else if (knapp_egenskaper[i][i2] == -2 && i2 == 1)
            {
                egenskaper[1] = last[1] + last[3] + 15;
                last[1] = egenskaper[1];
                knapp_egenskaper[i][i2] = egenskaper[i2];
            }
        }

        SDL_Rect knapp = { egenskaper[0], egenskaper[1], egenskaper[2], egenskaper[3] };
        SDL_RenderFillRect(renderer, &knapp);
        
        if (!knapp_textures[i])
        {
            fprintf(stderr, "Det oppsto en error ved tekst initialisering %s \n", SDL_GetError());
        }
        else
        {
            SDL_Rect destinasjon = { egenskaper[0] + (egenskaper[2] / 1.5) - knapp_surfaces[i]->w, egenskaper[1], knapp_surfaces[i]->w, knapp_surfaces[i]->h};
            SDL_RenderCopy(renderer, knapp_textures[i], NULL, &destinasjon);
        }
    }

    SDL_RenderPresent(renderer);
}

void quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(vindu);
    TTF_CloseFont(standard_font);
    TTF_Quit();
    SDL_Quit();
    if (lengde >0)
    {
        free(nummerene);
        free(operasjoner);
    }
}

int main(int argc, char** args)
{
    loop = initialiser_vindu();

    setup();

    while (loop)
    {
        inndata_prosessering();
        oppdater();
        render();
    }
    quit();
    
    return 0;
}

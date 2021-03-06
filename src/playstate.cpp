#include"include/playstate.h"


#include"include/game_engine.h"
#include"include/tetris.h"
#include"include/board.h"
#include"include/reuse.h"
#include"random"

PlayState PlayState::m_playstate;
static bool lose_music = false;


void PlayState::init(GameEngine* game){

    board                   = new Board();
    tetris                  = new Tetris(static_cast<int>(random()%7));
    next_tetris             = new Tetris(static_cast<int>(random()%7));

    //砖块材质加载
    block_texture           = loadTexture("resource/img/block1.bmp", game->renderer);

    //背景加载
    background_texture      = loadTexture("resource/img/bg.png", game->renderer);

    //声音引擎加载
    music_engine            = irrklang::createIrrKlangDevice();
    music_engine            ->play2D("resource/sounds/bgm.ogg", true);
    music_engine            ->setSoundVolume(0.4);
    sound_engine            = irrklang::createIrrKlangDevice();
    lose_engine             = irrklang::createIrrKlangDevice();

    //字体载入
    TTF_Init();
    white                   = {255, 255, 255};
    font_tetris             = TTF_OpenFont("resource/fonts/MonsterFriendFore.otf", 16);
    font_score_text         = TTF_OpenFont("resource/fonts/DTM-Mono.otf", 20);
    font_score              = TTF_OpenFont("resource/fonts/DTM-Sans.otf", 20);

    font_pause              = TTF_OpenFont("resource/fonts/DTM-Sans.otf", 16);
    font_game_over          = TTF_OpenFont("resource/fonts/DTM-Mono.otf", 26);

    font_image_pause        = renderText("PAUSE"    , white, font_pause     , game->renderer);
    font_image_game_over    = renderText("GAME OVER", white, font_game_over , game->renderer);
    font_image_tetris       = renderText("TETRIS"   , white, font_tetris    , game->renderer);
    font_image_score_text   = renderText("SCORE"  , white, font_score_text, game->renderer);
    font_image_score        = renderText(std::to_string(board->getScore())  , white, font_score, game->renderer);

    acceleration            = 0.005f;
    this_time               = 0;
    last_time               = 0;
    time_till_drop          = 0.3f;
    time_counter            = 0.0f;

    newgamedown             = false;
    newgameup               = false;
    quitdown                = false;
    quitup                  = false;

    newgamex1               = GAME_OFFSET+Board::WINDOW_WIDTH+Board::WTH_PER_BLOCK;
    newgamex2               = GAME_OFFSET+Board::WINDOW_WIDTH+8*Board::WTH_PER_BLOCK;
    newgamey1               = Board::WINDOW_HEIGHT-4*Board::HEI_PER_BLOCK;
    newgamey2               = Board::WINDOW_HEIGHT-6*Board::HEI_PER_BLOCK;

    //游戏状态
    paused          = false;
    game_over       = false;
    exit            = false;

    tetris->setPoint(static_cast<int>(Board::COLS/2), 0);

    next_tetris->setPoint(Board::COLS+5, static_cast<int>(0.3*Board::ROWS));

}


void PlayState::clean_up(GameEngine* game){
    music_engine->drop();
    sound_engine->drop();
    lose_engine->drop();

    TTF_CloseFont(font_tetris);
    TTF_CloseFont(font_score);
    TTF_CloseFont(font_score_text);
    TTF_CloseFont(font_pause);
    TTF_CloseFont(font_game_over);

    SDL_DestroyTexture(font_image_pause);
    SDL_DestroyTexture(font_image_tetris);
    SDL_DestroyTexture(font_image_score_text);
    SDL_DestroyTexture(font_image_score);
    SDL_DestroyTexture(font_image_game_over);
    SDL_DestroyTexture(background_texture);

    IMG_Quit();

    SDL_DestroyWindow(game->window);
    SDL_DestroyRenderer(game->renderer);
    SDL_Quit();
}

void PlayState::pause(){
    music_engine->setAllSoundsPaused(true);
    paused = true;
}

void PlayState::resume(){
    music_engine->setAllSoundsPaused(false);
    paused = false;
}

void PlayState::reset(){
    for(auto & i : board->color)
        for(int & j : i)
            j = -1;

    //删除旧有的对象
    delete [] board;
    delete [] tetris;
    delete [] next_tetris;

    //重新创建对象
    board = new Board();
    tetris = new Tetris(rand()%7);
    next_tetris = new Tetris(rand()%7);

    tetris->setPoint(static_cast<int>(Board::COLS/2), 0);
    next_tetris->setPoint(Board::COLS+5, static_cast<int>(0.3*Board::ROWS));

    //音频重置
    music_engine->stopAllSounds();
    music_engine->play2D("resource/sounds/bgm.ogg", true);
    music_engine            ->setSoundVolume(0.4);

    //游戏状态重置
    game_over            = false;
    newgameup            = false;
    newgamedown          = false;
    lose_music           = false;

    paused = false;

}
void PlayState::input(GameEngine* game){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        //退出事件
        if(event.type == SDL_QUIT){
            exit = true;
        }

        //键盘按下事件
        if(event.type == SDL_KEYDOWN){
            if(event.key.keysym.sym == SDLK_p){
                if(paused)      resume();
                else            pause(); }

            //方块操作
        if(!paused && !tetris->fall){
            switch(event.key.keysym.sym){
            case SDLK_ESCAPE:
                exit = true; break;
            // a 方向键左: 左移动
            case SDLK_a: case SDLK_LEFT:
                tetris->movement = tetris->LEFT;
                tetris->shift = true; break;
            // d 方向键右: 右移动
            case SDLK_d: case SDLK_RIGHT:
                tetris->movement = tetris->RIGHT;
                tetris->shift = true; break;
            // w 方向键上: 方块旋转(O型方块除外)
            case SDLK_w: case SDLK_UP:
                if (tetris->type != 2) //O
                    tetris->rotate = true;
                break;
            // s 方向键下: 方块加速下落
            case SDLK_s: case SDLK_DOWN:
                tetris->speedup = true; break;
            // 空格键: 方块直接落地
            case SDLK_SPACE:
                tetris->fall = true;
            default: break;
            } }
        }


        //键盘松开事件
        if(event.type == SDL_KEYUP){
            switch(event.key.keysym.sym){
            case SDLK_s: case SDLK_DOWN:
                tetris->speedup = false; break;
            default: break;
            }
        }

        //鼠标移动事件
        if(event.type == SDL_MOUSEMOTION){
            //方块池外
            if(event.motion.x > Board::WINDOW_WIDTH + GAME_OFFSET)
                SDL_ShowCursor(1);      //显示光标(方块池外)
            //方块池内
            else
                SDL_ShowCursor(0);      //隐藏光标(方块池内)
        }

        //鼠标点击事件
        if(event.type == SDL_MOUSEBUTTONDOWN){
            switch(event.button.button){
            case SDL_BUTTON_LEFT: {
            int x = event.button.x;
            int y = event.button.y;
            if (x > newgamex1 && x < newgamex2){
                if(y > newgamey2 && y < newgamey1)
                    //光标置于"NEW GAME" 位置
                    { newgamedown = true; }
                else if (y > newgamey2 + 4*Board::HEI_PER_BLOCK &&
                         y < newgamey1 + 4*Board::HEI_PER_BLOCK){
                    //光标置于"QUIT"位置
                    quitdown = true; } } break; }
            default: break; } }

        //鼠标松开事件
        if(event.type == SDL_MOUSEBUTTONDOWN){ switch(event.button.button){
            case SDL_BUTTON_LEFT: {
            int x = event.button.x;
            int y = event.button.y;
            if (x > newgamex1 && x < newgamex2){
                if(y > newgamey2 && y < newgamey1)
                    //光标置于"NEW GAME" 位置
                    newgameup = true;
                else if (y > newgamey2 + 4*Board::HEI_PER_BLOCK &&
                         y < newgamey1 + 4*Board::HEI_PER_BLOCK){
                    //光标置于"QUIT"位置
                    quitup = true; } } break; }
            default: break;
            }
        }
    }
}

void PlayState::releaseBlocks(){
    auto* new_tetris = new Tetris(rand()%7);
    new_tetris->setPoint(next_tetris->x, next_tetris->y);

    delete[] tetris;
    tetris = next_tetris;
    tetris->setPoint(static_cast<int>(Board::COLS/2), 0);

    next_tetris = new_tetris;

    tetris->drop();
}

void PlayState::update(GameEngine* game){
    //按下NEW GAME
    if(newgameup && newgamedown)
        reset();

    // 按下QUIT
    if((quitup && quitdown) || exit)
        game->quit();

    if(game_over || paused)
        return;

    // 方块落地时处理
    if (tetris->has_landed()){
        tetris->fall = false;
        if(!board->add(tetris)){
            game_over = true;
            return; }
        releaseBlocks();
    }else if(tetris->fall){
        tetris->y++;//加速
    }else{
        if(tetris->rotate)
            tetris->rotateLeft();
        tetris->add_to_x(tetris->movement);

        time_till_drop = tetris->speedup ? 0.02f : 0.3f - board->getScore() * acceleration;

        time_counter += frame_rate(game, &last_time, &this_time);

        if(time_counter >= time_till_drop){
            tetris->y++;
            time_counter = 0.0f;
        }
    }

    for(int i = 0; i < Tetris::SIZE; i++){
        int x = tetris->getX(i);
        int y = tetris->getY(i);

        if(x < 0 || x >= Board::COLS){
            if(tetris->rotate)
                tetris->rotateRight();

            if(tetris->shift)
                tetris->x -= tetris->movement;
            break;

        }else if(y >= Board::ROWS){
            tetris->lands();

            tetris->setY(i, Board::ROWS-1);
        }else if(y >= 0){
            if(board->color[y][x] != -1){
                if(tetris->rotate || tetris->shift){
                    if(tetris->rotate){
                        tetris->rotateRight();
                    }
                    if(tetris->shift){
                        tetris->x -= tetris->movement;
                    }
                    break;
                }else{
                    tetris->y--;
                    tetris->lands();
                }
            }
        }
    }
    int bonus;
    bonus = board->letItGo();
    switch (bonus){
        case(0): sound_engine->play2D("resource/sounds/1.ogg", false);break;
        case(1): sound_engine->play2D("resource/sounds/2.ogg", false);break;
        case(2): sound_engine->play2D("resource/sounds/3.ogg", false);break;
        case(3): sound_engine->play2D("resource/sounds/4.ogg", false);break;
        case(-1):break;
    }

    tetris->rotate = false;
    tetris->shift = false;
    tetris->movement = tetris->NONE;
}

void PlayState::render(GameEngine* game){
    SDL_SetRenderDrawColor(game->renderer, 0, 1, 0, 1);
    SDL_RenderClear(game->renderer);

    SDL_SetTextureAlphaMod(background_texture, 100);
    renderTexture(background_texture, game->renderer, 0, 0);

    //渲染 "TETRIS" 文字
    int x = (next_tetris->x-3) * Board::WTH_PER_BLOCK;
    int y = GAME_OFFSET;

    renderTexture(font_image_tetris, game->renderer, x, y);
    //渲染 "PAUSE" 文字
    if(paused)
        renderTexture(font_image_pause, game->renderer, x, y+40);
    //渲染"SCORE"文字
    renderTexture(font_image_score_text, game->renderer, x, y + Board::WTH_PER_BLOCK);
    //渲染分数
    if(board->render_score){
        font_image_score = renderText(std::to_string(board->getScore()),
                                      white, font_score, game->renderer);
        board->render_score = false;
    }
    renderTexture(font_image_score,
                  game->renderer, x + 60, y + Board::WTH_PER_BLOCK);

    int  block_x, block_y;

    int iW, iH;
    SDL_QueryTexture(block_texture, nullptr, nullptr, &iW, &iH);

    SDL_Rect clips[7];
    for(int i = 0; i < 7; i++){
        clips[i].x = 0;
        clips[i].y = i*24;
        clips[i].w = 20;
        clips[i].h = 20;
    }

    for(int i = 0; i < Tetris::SIZE; i++){
        block_x = tetris->getX(i)*Board::WTH_PER_BLOCK + GAME_OFFSET;
        block_y = tetris->getY(i)*Board::WTH_PER_BLOCK + GAME_OFFSET;

        drawBlock(game, block_x, block_y, tetris->type, clips);
    }

    int shadow_y[4];
    tetris->getShadow(board, shadow_y);
    for(int i = 0; i < Tetris::SIZE; i++){
        if(shadow_y[i] < 0)
            break;
        int x = tetris->getX(i)*Board::WTH_PER_BLOCK + GAME_OFFSET;
        int y = shadow_y[i]*Board::WTH_PER_BLOCK + GAME_OFFSET;

        SDL_SetRenderDrawColor(game->renderer, 180, 180, 180, 255);
        SDL_Rect shadow_block = {x, y, Board::WTH_PER_BLOCK, Board::HEI_PER_BLOCK};
        SDL_RenderFillRect(game->renderer, &shadow_block);
    }

    if(!game_over){
        for(int i = 0; i < Tetris::SIZE; i++){
            block_x = next_tetris->getX(i)*Board::WTH_PER_BLOCK;
            block_y = next_tetris->getY(i)*Board::HEI_PER_BLOCK;

            drawBlock(game, block_x, block_y, next_tetris->type, clips);
        }
    }
    //方块池待命方块
    for(int i = 0; i < Board::ROWS; i++)
        for(int j = 0; j < Board::COLS; j++)
            if(board->color[i][j] != -1) {
                block_x = j*Board::WTH_PER_BLOCK + GAME_OFFSET;
                block_y = i*Board::HEI_PER_BLOCK + GAME_OFFSET;

                drawBlock(game, block_x, block_y, board->color[i][j], clips);
            }

    SDL_SetRenderDrawColor(game->renderer, 180, 180, 180, 255);

    //方块池左边缘
    SDL_RenderDrawLine(game->renderer,
                       GAME_OFFSET, GAME_OFFSET,
                       GAME_OFFSET, GAME_OFFSET+Board::WINDOW_HEIGHT);
    //方块池右边缘
    SDL_RenderDrawLine(game->renderer,
                       GAME_OFFSET+Board::WINDOW_WIDTH, GAME_OFFSET,
                       GAME_OFFSET+Board::WINDOW_WIDTH, GAME_OFFSET+Board::WINDOW_HEIGHT);

    //方块池顶边缘
    SDL_RenderDrawLine(game->renderer,
                       GAME_OFFSET, GAME_OFFSET,
                       GAME_OFFSET+Board::WINDOW_WIDTH, GAME_OFFSET);

    //方块池底边缘
    SDL_RenderDrawLine(game->renderer,
                       GAME_OFFSET, GAME_OFFSET+Board::WINDOW_HEIGHT,
                       GAME_OFFSET+Board::WINDOW_WIDTH, GAME_OFFSET+Board::WINDOW_HEIGHT);

    if (game_over)
    {
        if(!lose_music)
        {
            lose_engine ->play2D("resource/sounds/lose.ogg");
            lose_music  = true;
        }

        music_engine->setAllSoundsPaused(true);
        renderTexture(font_image_game_over,
                      game->renderer, newgamex1,
                      game->height-newgamey1+4*Board::WTH_PER_BLOCK);
    }

    //显示内容
    SDL_RenderPresent(game->renderer);
}



void PlayState::drawBlock(GameEngine* game,
                          int x, int y, int k, SDL_Rect clips[]){
    renderTexture(block_texture, game->renderer, x, y, &clips[k]);
}


float PlayState::frame_rate(GameEngine* game, int* last_time, int* this_time){
    *last_time = *this_time;

    *this_time = SDL_GetTicks();

    return ((*this_time - *last_time) / 1000.0f);
}

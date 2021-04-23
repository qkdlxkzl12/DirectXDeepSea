#include"gameData.h"

BACKGROUND g_bgIngame[2][4] = { { BACKGROUND(), }, };

PLAYER g_player = PLAYER(0, 0, 87, 34);
ENEMY enemy = ENEMY(100,10,0,0,153, 83);
HRESULT InitD3D( HWND hWnd )
{
    // Create the D3D object, which is needed to create the D3DDevice.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    // Device state would normally be set here

    return S_OK;
}

VOID Cleanup()
{
    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

VOID Update()
{
    GameUpdate();
}

VOID Render()
{
    if( NULL == g_pd3dDevice )
        return;

    // Clear the backbuffer to a blue color
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // Rendering of scene objects can happen here
        g_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
        GameRender();
        g_pSprite->End();
        // End the scene
        g_pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
    UNREFERENCED_PARAMETER( hInst );

    // Register the window class
    WNDCLASSEX wc =
    {
        sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
        L"D3D Tutorial", NULL
    };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 01: DeepSea",
                              WS_POPUP, 50, 50, SCREEN_WIDTH, SCREEN_HEIGHT,
                              NULL, NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Show the window
        ShowWindow( hWnd, SW_SHOWDEFAULT );
        UpdateWindow( hWnd );
        GameInit();
        // Enter the message loop
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while( msg.message != WM_QUIT )
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                g_curTime = GetTickCount64();
                    Update();
                    Render();
            }
        }
    }

    UnregisterClass( L"D3D Tutorial", wc.hInstance );
    return 0;
}

VOID GameInit(){
    LPDIRECT3DTEXTURE9 texture = NULL;
    D3DXCreateSprite(g_pd3dDevice, &g_pSprite);
    //Init to Background "g_bgIngame"
    {
        OBJECT::LoadTexture(L"Resources/BG_1.png", &texture);
        g_bgIngame[0][0].SetTexture(texture);
        g_bgIngame[0][0].SetMoveSpeed(0);
        OBJECT::LoadTexture(L"Resources/BG_2.png", &texture);
        g_bgIngame[0][1].SetTexture(texture);
        g_bgIngame[0][1].SetMoveSpeed(3);
        OBJECT::LoadTexture(L"Resources/BG_3.png", &texture);
        g_bgIngame[0][2].SetTexture(texture);
        g_bgIngame[0][2].SetMoveSpeed(5);
        OBJECT::LoadTexture(L"Resources/BG_4.png", &texture);
        g_bgIngame[0][3].SetTexture(texture);
        g_bgIngame[0][3].SetMoveSpeed(8);
        for (int i = 0; i < 4; i++) {
            g_bgIngame[1][i].texture = g_bgIngame[0][i].texture;
            g_bgIngame[1][i].pos += { SCREEN_WIDTH, 0, 0};
            g_bgIngame[1][i].SetMoveSpeed(g_bgIngame[0][i].GetMoveSpeed());
        }
    }
    //Init to Player "g_Player"
    {
        OBJECT::LoadTexture(L"Resources/Player.png", &texture);
        g_player.SetTexture(texture);
        g_player.visible = TRUE;
        g_player.bullet[0].SetTexture(texture);
        for (int i = 0; i < 100; i++)
            g_player.bullet[i].texture = g_player.bullet[0].texture;
    }
    //Inite to Enemy 
    {
        OBJECT::LoadTexture(L"Resources/enemy1.png", &texture);
        enemy.SetTexture(texture);
        enemy.visible = true;
    }
}

VOID GameRender(){
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 2; j++)
    g_bgIngame[j][i].Draw();;
    g_player.Draw();
    for (int i = 0; i < 100; i++)
        g_player.bullet[i].Draw();
    enemy.Draw();
}

VOID GameUpdate() {
    for (int j = 0; j < 2; j++)
        for (int i = 1; i < 4; i++)
            g_bgIngame[j][i].MoveBackground();

    g_player.pos.x -= 2;
    g_player.Control();
    g_player.OutedBorder();
    for (int i = 0; i < 100; i++)
    {
        g_player.bullet[i].Fired();
        if (g_player.bullet[i].visible)
            if (OnHit(g_player.bullet[i], enemy))
            {
                enemy.GetDamage(5);
                g_player.bullet[i].Outed();
            }
    }
    enemy.ChangeColor();
}
VOID GameRelease() {

}

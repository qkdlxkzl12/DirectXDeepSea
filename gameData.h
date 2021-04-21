#include <d3d9.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)       ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)
#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900

LPDIRECT3D9         g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9   g_pd3dDevice = NULL; // Our rendering device

LPD3DXSPRITE g_pSprite = NULL; 

time_t g_curTime;

class TIME {
private:
	time_t oldTime;
	INT delay;
public:
	TIME()
	{
		oldTime = 0;
		delay = 0;
	}

	TIME(INT delay)
	{
		oldTime = 0;
		TIME::delay = delay;
	}

	BOOL IsEnoughPassed()
	{
		if (g_curTime - oldTime < delay)
			return FALSE;
		oldTime = g_curTime;
		return TRUE;
	}
};

class OBJECT {
public:
	static VOID LoadTexture(const wchar_t* Resource, LPDIRECT3DTEXTURE9* texture)
	{
		D3DXCreateTextureFromFileEx(g_pd3dDevice, Resource, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, 0, 0, 0, 0, texture);
	}

	BOOL visible;
	LPDIRECT3DTEXTURE9 texture;
	RECT rect;
	D3DXVECTOR3 center;
	D3DXVECTOR3 pos;
	INT color;
	//持失切
	OBJECT()
	{
		visible = TRUE;
		texture = NULL;
		rect = { 0, 0,  SCREEN_WIDTH, SCREEN_HEIGHT };
		center = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0 };
		pos = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0 };
		color = 0xffffffff;
	}

	OBJECT(INT startPosX, INT startPosY, INT width, INT height) //持失切
	{
		visible = FALSE;
		texture = NULL;
		rect = { startPosX, startPosY, startPosX + width, startPosY + height };
		center = { width * 0.5f, height * 0.5f, 0 };
		pos = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0 };
		color = 0xffffffff;
	}
	/// ////////////////////////////////////////////
	VOID Draw()
	{
		if(visible)
		g_pSprite->Draw(texture, &rect, &center, &pos, color);
	}

	VOID SetTexture(LPDIRECT3DTEXTURE9 texture)
	{
		OBJECT::texture = texture;
	}

	INT GetHalfWidth()
	{
		return center.x;
	}

	INT GetHalfHeight()
	{
		return center.y;
	}
};

class ACTOR : public OBJECT
{
private:
	INT maxHealth;
	INT currentHealth;
	INT moveSpeed;

public:
	ACTOR() : OBJECT()
	{
		maxHealth = 1;
		currentHealth = maxHealth;
		moveSpeed = 1;
	}

	ACTOR(INT health,INT moveSpeed, INT startPosX, INT startPosY, INT width, INT height) : OBJECT( startPosX,  startPosY,  width,  height)
	{
		maxHealth = health;
		currentHealth = maxHealth;
		ACTOR::moveSpeed = moveSpeed;
	}


	VOID MoveLeft()
	{
		pos.x -= moveSpeed;
	}
	VOID MoveRight()
	{
		pos.x += moveSpeed;
	}
	VOID MoveUp()
	{
		pos.y -= moveSpeed;
	}
	VOID MoveDown()
	{
		pos.y += moveSpeed;
	}

	INT GetHealth()
	{
		return currentHealth;
	}

	VOID AddHealth(INT value)
	{
		currentHealth += value;
	}

	VOID GetDamage(INT damage)
	{
		currentHealth -= damage;
		color = 0xffaa0000;
		
	}

	VOID OnDie()
	{

	}
};

class BACKGROUND : public OBJECT
{
private:
	INT moveSpeed;

public:

	BACKGROUND()
	{
		OBJECT();
		moveSpeed = NULL;
	}

	VOID SetMoveSpeed(INT Value)
	{
		moveSpeed = Value;
	}

	INT GetMoveSpeed()
	{
		return moveSpeed;
	}

	VOID MoveBackground()
	{
		pos.x -= GetMoveSpeed();

		if (pos.x <= SCREEN_WIDTH * -0.5f)
			pos.x = SCREEN_WIDTH * 1.5f;
	}
};

class BULLET : public OBJECT
{
private:
	 D3DXVECTOR3 startPos;
	 INT type;
	 INT moveSpeed;
	 BOOL isFired;
public:
	BULLET() : OBJECT()
	{
		visible = FALSE;
		rect = { 100,0,110,6 };
		center = { 5,3,0 };
		startPos = { 0,0,0 };
		type = 0;
		moveSpeed = 5;
		isFired = FALSE;
	}

	VOID SetStartPos(D3DXVECTOR3 vector3)
	{
		startPos = vector3;
	}

	VOID Init( )
	{
		pos = startPos;
		isFired = TRUE;
	}

	VOID Fired()
	{
		if (!isFired)
			return;
		pos.x += moveSpeed;
		if (pos.x - GetHalfWidth() > SCREEN_WIDTH)
		{
			isFired = FALSE;
			visible = FALSE;
		}
	}

	VOID ChangeType(INT type)
	{
		BULLET::type = type;
		rect = { 100,10 * type,110,10 * type + 6 };
	}
};

class PLAYER : public ACTOR
{
private:
	BOOL isShoted;
	INT attackType;
	TIME tFiring;
	TIME tChangeT;
	
public:
	BULLET bullet[100] = { BULLET(), };

	PLAYER(INT startPosX, INT startPosY, INT width, INT height) : ACTOR(100, 5, startPosX, startPosY, width, height)
	{
		isShoted = FALSE;
		attackType = 0;
		tFiring = TIME(100);
		tChangeT = TIME(1000);
	}

	VOID Control()
	{
		if (KEY_DOWN(VK_LEFT))
			if(pos.x - GetHalfWidth() > 0)
			MoveLeft();
		if (KEY_DOWN(VK_RIGHT))
			if (pos.x + GetHalfWidth() < SCREEN_WIDTH)
			MoveRight();
		if (KEY_DOWN(VK_UP))
			if (pos.y - GetHalfHeight() > 0)
			MoveUp();
		if (KEY_DOWN(VK_DOWN))
			if (pos.y + GetHalfHeight() < SCREEN_HEIGHT)
			MoveDown();
		if (KEY_DOWN(VK_SPACE))
			ShotBullet();
		if (KEY_DOWN(INT('X')))
			ChangeAttackType();
	}

	VOID OutedBorder()
	{
		if (pos.x - GetHalfWidth() < -20)
		{
			GetDamage(10);
			pos.x = GetHalfWidth() + 30;
		}
	}

	VOID ShotBullet()
	{
		if(tFiring.IsEnoughPassed())
		{ 
			for (int i = 0; i < 100; i++)
			{
				if (bullet[i].visible)
					continue;
				bullet[i].visible = TRUE;
				bullet[i].SetStartPos({ pos.x + bullet[i].GetHalfWidth() + GetHalfWidth() - 5, pos.y + bullet[i].GetHalfHeight() + 6 ,0 });
				bullet[i].ChangeType(attackType);
				bullet[i].Init();
				break;
			}
		}
	}

	VOID ChangeAttackType()
	{
		if(tChangeT.IsEnoughPassed())
		++attackType %= 4;
	}
	
};

VOID GameInit();
VOID GameRender();
VOID GameUpdate();
VOID GameRelease();
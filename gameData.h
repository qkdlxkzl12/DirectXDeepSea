#include <d3d9.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#include <coroutine>
#include <list>
#include<cassert>
#include<ctime>
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

	BOOL IsEnoughPassed(BOOL usedResetOldtime)
	{
		if (g_curTime - oldTime < delay)
			return FALSE;
		if (usedResetOldtime == TRUE)
			InitOltime();
		return TRUE;
	}
	VOID InitOltime()
	{
		oldTime = g_curTime;
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
	//생성자
	OBJECT()
	{
		visible = TRUE;
		texture = NULL;
		rect = { 0, 0,  SCREEN_WIDTH, SCREEN_HEIGHT };
		center = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0 };
		pos = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0 };
		color = 0xffffffff;
	}
	OBJECT(INT startPosX, INT startPosY, INT width, INT height) //생성자
	{
		visible = FALSE;
		texture = NULL;
		rect = { startPosX, startPosY, startPosX + width, startPosY + height };
		center = { width * 0.5f, height * 0.5f, 0 };
		pos = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0 };
		color = 0xffffffff;
	}
	VOID Draw()
	{
		if (visible)
			g_pSprite->Draw(texture, &rect, &center, &pos, color);
	}

	VOID SetTexture(LPDIRECT3DTEXTURE9 texture)
	{
		this->texture = texture;
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

class ANIMATION
{
private:
	INT animMaxCount;
	INT animCount;
	TIME tAnim;
	OBJECT *obj;
public:
	ANIMATION()
	{	}
	ANIMATION(INT count, INT delday, OBJECT* obj) : animMaxCount(count), obj(obj)
	{
		animCount = 0;
		tAnim = TIME(delday);
	}

	VOID PlayAnim()
	{
		if (tAnim.IsEnoughPassed(TRUE) == FALSE)
			return;
		int a = obj->rect.right - obj->rect.left;
		obj->rect.left = animCount * a;
		obj->rect.right = (animCount + 1) * a;
		++animCount %= animMaxCount;
	}
};

class UI : public OBJECT
{
public:
	UI(INT startPosX, INT startPosY, INT width, INT height) : OBJECT(startPosX, startPosY, width, height)
	{

	};

};

class ACTOR : public OBJECT
{
private:
	TIME tOnHit;
	INT maxHealth;
	INT currentHealth;
	INT moveSpeed;

public:
	ACTOR() : OBJECT()
	{
		maxHealth = 1;
		currentHealth = maxHealth;
		moveSpeed = 1;
		tOnHit = TIME(60);
	}

	ACTOR(INT health,INT moveSpeed, INT startPosX, INT startPosY, INT width, INT height) : OBJECT( startPosX,  startPosY,  width,  height)
	{
		maxHealth = health;
		currentHealth = maxHealth;
		ACTOR::moveSpeed = moveSpeed;
		tOnHit = TIME(60);
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

	VOID MoveLeft(INT value)
	{
		pos.x -= value;
	}
	VOID MoveRight(INT value)
	{
		pos.x += value;
	}
	VOID MoveUp(INT value)
	{
		pos.y -= value;
	}
	VOID MoveDown(INT value)
	{
		pos.y += value;
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
		tOnHit.InitOltime();
		
	}

	VOID ChangeColor()
	{
		color = 0xffffffff; 
		if (tOnHit.IsEnoughPassed(FALSE) == FALSE)
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

	VOID SetStartPos(D3DXVECTOR3 vector3);
	VOID Init();
	VOID Fired();
	VOID ChangeType(INT type);
	VOID Outed();
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

	VOID Control();
	VOID OutedBorder();
	VOID ShotBullet();
	VOID ChangeAttackType();
};

class ENEMY : public ACTOR
{
public:
	ENEMY() {};
	ENEMY(INT health, INT moveSpeed, INT startPosX, INT startPosY, INT width, INT height) : ACTOR(health, moveSpeed, startPosX, startPosY, width, height)
	{

	}

	ANIMATION moveAnim;
	virtual VOID Move() = 0;
};

class ENEMY1 : public ENEMY
{
private:
	INT movementSize;
	INT movement;
public:
	VOID Init();
	ENEMY1(INT health, INT moveSpeed, INT startPosX, INT startPosY, INT width, INT height) : ENEMY(health, moveSpeed, startPosX, startPosY, width, height) 
	{
	moveAnim = ANIMATION(6, 100, this);
	};
	VOID HitWithBullet(BULLET* bullet);
	VOID Move() override;

};

class ENEMY2 : public ENEMY
{
private:
public :
	ENEMY2(INT health, INT moveSpeed, INT startPosX, INT startPosY, INT width, INT height) : ENEMY(health, moveSpeed, startPosX, startPosY, width, height) 
	{
		moveAnim = ANIMATION(4, 70, this);
	};
	VOID Move() override;

};

VOID GameInit();
VOID GameRender();
VOID GameUpdate();
VOID GameRelease();

VOID PlayerUpdate();
VOID EnemyUpdate();
template<class C1, class C2>
BOOL OnHit(C1 obj1, C2 obj2)
{
	//FLOAT L1 = obj1.pos.x - obj1.GetHalfWidth();
	//FLOAT R1 = obj1.pos.x + obj1.GetHalfWidth();
	//FLOAT L2 = obj2.pos.x - obj2.GetHalfWidth();
	//FLOAT R2 = obj2.pos.x + obj2.GetHalfWidth();
	//if ((L1 - R2)*(L2 - R1) < 0) //왼점에서 오른점 뺀게 부호가 다르면 비충돌
	//	return FALSE;
	//L1 = obj1.pos.y - obj1.GetHalfHeight();
	//R1 = obj1.pos.y + obj1.GetHalfHeight();
	//L2 = obj2.pos.y - obj2.GetHalfHeight();
	//R2 = obj2.pos.y + obj2.GetHalfHeight();
	//if ((L1 - R2) * (L2 - R1) < 0) //윗점에서 아랫점 뺀게 부호가 다르면 비충돌
	//	return FALSE;
	//return TRUE;
	return FALSE;
}
template<>
BOOL OnHit<BULLET,ENEMY1>(BULLET obj1, ENEMY1 obj2)
{
	FLOAT L1 = obj1.pos.x - obj1.GetHalfWidth();
	FLOAT R1 = obj1.pos.x + obj1.GetHalfWidth();
	FLOAT L2 = obj2.pos.x - obj2.GetHalfWidth();
	FLOAT R2 = obj2.pos.x + obj2.GetHalfWidth();
	if ((L1 - R2)*(L2 - R1) < 0) //왼점에서 오른점 뺀게 부호가 다르면 비충돌
		return FALSE;
	L1 = obj1.pos.y - obj1.GetHalfHeight();
	R1 = obj1.pos.y + obj1.GetHalfHeight();
	L2 = obj2.pos.y - obj2.GetHalfHeight();
	R2 = obj2.pos.y + obj2.GetHalfHeight();
	if ((L1 - R2) * (L2 - R1) < 0) //윗점에서 아랫점 뺀게 부호가 다르면 비충돌
		return FALSE;
	return TRUE;
}
template<>
BOOL OnHit<PLAYER, ENEMY1>(PLAYER obj1, ENEMY1 obj2)
{
	FLOAT L1 = obj1.pos.x - obj1.GetHalfWidth();
	FLOAT R1 = obj1.pos.x + obj1.GetHalfWidth();
	FLOAT L2 = obj2.pos.x - obj2.GetHalfWidth();
	FLOAT R2 = obj2.pos.x + obj2.GetHalfWidth();
	if ((L1 - R2) * (L2 - R1) < 0) //왼점에서 오른점 뺀게 부호가 다르면 비충돌
		return FALSE;
	L1 = obj1.pos.y - obj1.GetHalfHeight();
	R1 = obj1.pos.y + obj1.GetHalfHeight();
	L2 = obj2.pos.y - obj2.GetHalfHeight();
	R2 = obj2.pos.y + obj2.GetHalfHeight();
	if ((L1 - R2) * (L2 - R1) < 0) //윗점에서 아랫점 뺀게 부호가 다르면 비충돌
		return FALSE;
	return TRUE;
}

// / BULLET FUNCTION / //
VOID BULLET::SetStartPos(D3DXVECTOR3 vector3)
{
	startPos = vector3;
}

VOID BULLET::Init()
{
	pos = startPos;
	isFired = TRUE;
}

VOID BULLET::Fired()
{
	if (!isFired)
		return;
	pos.x += moveSpeed;
	if (pos.x - GetHalfWidth() > SCREEN_WIDTH)
		Outed();
}

VOID BULLET::ChangeType(INT type)
{
	BULLET::type = type;
	rect = { 100,10 * type,110,10 * type + 6 };
}

VOID BULLET::Outed()
{
	isFired = FALSE;
	visible = FALSE;
}


// / PLAYER FUNCTION / //
VOID PLAYER::Control()
{
	if (KEY_DOWN(VK_LEFT))
		if (pos.x - GetHalfWidth() > 0)
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

	ChangeColor();
}

VOID PLAYER::OutedBorder()
{
	pos.x -= 2;
	if (pos.x - GetHalfWidth() < -20)
	{
		GetDamage(10);
		pos.x = GetHalfWidth() + 30;
	}
}

VOID PLAYER::ShotBullet()
{
	if (tFiring.IsEnoughPassed(TRUE) == FALSE)
		return;
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

VOID PLAYER::ChangeAttackType()
{
	if (tChangeT.IsEnoughPassed(TRUE))
		++attackType %= 4;
}

// / ENEMY FUNCTION / //
VOID ENEMY1::HitWithBullet(BULLET* bullet)
{
	if (bullet->visible == TRUE)
		if (OnHit(bullet, *this))
		{
			this->GetDamage(5);
			bullet->Outed();
		}
	//if (bullet->visible == FALSE)
	//	return;
	//if (OnHit(&bullet, this) == FALSE)
	//	return;
	//this->GetDamage(5);
	//bullet->Outed();
}

VOID ENEMY1::Move()
{
	MoveLeft();
	MoveUp(INT(movementSize * sinf(0.1 / movementSize * pos.x)));
}
VOID ENEMY1::Init()
{
	moveAnim = ANIMATION(6, 100, this);
	movementSize = 3 + rand() % 5;
	
}
VOID ENEMY2::Move(){
	return;
}
// / CUSTOM FUNCTION / //
VOID AddEnemy(INT type);
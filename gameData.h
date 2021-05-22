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

struct STATE {
	INT max;
	INT current;
};

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
	UI(){}
	UI(INT startPosX, INT startPosY, INT width, INT height) : OBJECT(startPosX, startPosY, width, height)
	{

	};
};

typedef class SKILL_UI : public UI
{
private:
	RECT actRect;
	RECT unActRect;
	INT costMana;
	TIME tCooltime;
	//활성화 스킬 관련 변수
	BOOL isActive;
	TIME tActive;
public:
	SKILL_UI(INT startPosX, INT startPosY, INT width, INT height) : UI(startPosX, startPosY, width, height)
	{
		BOOL isActive = FALSE;
		RECT actRect = {0,};
		RECT unActRect{0,};
		INT costMana = 0;
	}

	VOID Init(LPDIRECT3DTEXTURE9 tex, INT i)
	{
		texture = tex;
		visible = TRUE;
		pos = { pos.x + FLOAT(144 + (i * 120) - GetHalfWidth()), pos.y + 55 - GetHalfHeight(), 0 };
		actRect = rect;
		unActRect = actRect;
		unActRect.top += 100;
		unActRect.bottom += 100;
		switch (i)
		{
		case 0: 
			tCooltime = TIME(5000);
			costMana = 2;
			tActive = TIME(5000);
			isActive = TRUE;
			break;
		case 1:
			tCooltime = TIME(1000);
			costMana = 1;
			break;
		case 2:
			tCooltime = TIME(3000);
			costMana = 3;
			tActive = TIME(3000);
			isActive = TRUE;
			break;
		case 3:
			tCooltime = TIME(5000);
			costMana = 3;
			break;
		}
	}

	VOID ChangeImage(BOOL activation)
	{
		if (activation == TRUE)
			rect = actRect;
		else
			rect = unActRect;
	}
	BOOL IsCooltime()
	{
		if (isActive == TRUE)
			if (tActive.IsEnoughPassed(FALSE) == FALSE)
				tCooltime.InitOltime();
		if (tCooltime.IsEnoughPassed(FALSE) == FALSE)
		{
			ChangeImage(FALSE);
			return TRUE;
		}
		return FALSE;
	}
	BOOL IsLaskMana(INT mana)
	{
		if (mana - costMana < 0)
		{
			ChangeImage(FALSE);
			return FALSE;
		}
		return TRUE;
	}
	BOOL IsUseable(INT mana)
	{
		if (IsCooltime() == TRUE || IsLaskMana(mana) == FALSE)
			return FALSE;
		if (tCooltime.IsEnoughPassed(FALSE) == FALSE) //false = 쿨타임
			ChangeImage(FALSE);
		else
			ChangeImage(TRUE);
		return TRUE;
	}
	BOOL CanUse(INT mana)
	{
		if (IsLaskMana(mana) == FALSE || tCooltime.IsEnoughPassed(TRUE) == FALSE)
			return FALSE;
		return TRUE;
	}
	INT GetCost()
	{
		return costMana;
	}
	BOOL IsActive()
	{
		//액티브 스킬이 아니면 return
		if (isActive == FALSE)
			return FALSE;
		//활성화 되어있으면 쿨타임 줄지 않기 위해
		if (tActive.IsEnoughPassed(FALSE) == FALSE)
				tCooltime.InitOltime();
		return tActive.IsEnoughPassed(FALSE);
	}
	VOID InitActive()
	{
		tActive.InitOltime();
	}
}S_UI;

typedef class PLAYER_UI : public UI
{
private:
	S_UI uSkill[4] = { S_UI(0,200,64,57),S_UI(100,200,50,58),S_UI(200,200,45,47),S_UI(300,200,63,55) };
	UI uHealth[10] = { UI(0,400,36,36), };
	UI uEnergy[10] = { UI(50, 400, 36, 36), };
	INT* Pmana;
public:
	PLAYER_UI(INT startPosX, INT startPosY, INT width, INT height,INT* mana) : UI(startPosX, startPosY, width, height)
	{
		Pmana = mana;
	};

	VOID Draw()
	{
		OBJECT::Draw();
		for (INT i = 0; i < 4; i++)
			uSkill[i].Draw();
		for (INT i = 0; i < 10; i++)
			uEnergy[i].Draw();
		for (INT i = 0; i < 10; i++)
			uHealth[i].Draw();
	}
	VOID Init()
	{
		for (INT i = 0; i < 4; i++)
		{
			uSkill[i].Init(texture, i);
		}
		uSkill[0].pos = { pos.x - GetHalfWidth() + 144,pos.y - GetHalfHeight() + 57,0 };
		uSkill[1].pos = { pos.x - GetHalfWidth() + 257,pos.y - GetHalfHeight() + 57,0 };
		uSkill[2].pos = { pos.x - GetHalfWidth() + 375,pos.y - GetHalfHeight() + 57,0 };
		uSkill[3].pos = { pos.x - GetHalfWidth() + 500,pos.y - GetHalfHeight() + 60,0 };
		for (INT i = 0; i < 10; i++)
		{
			uEnergy[i] = uEnergy[0];
			uEnergy[i].texture = texture;
			uEnergy[i].visible = TRUE;
			uEnergy[i].pos = { pos.x - GetHalfWidth() + 507 + 33 * i, pos.y - GetHalfHeight() + 156,0 }; 
		}
		for (INT i = 0; i < 10; i++)
		{
			uHealth[i] = uHealth[0];
			uHealth[i].texture = texture;
			uHealth[i].visible = TRUE;
			uHealth[i].pos = { pos.x - GetHalfWidth() + 102 + 33 * i, pos.y - GetHalfHeight() + 156,0 }; 
		}
	}

	VOID SetHPnEP(std::pair<STATE, STATE> p)
	{
		STATE hp = p.first;
		STATE ep = p.second;

		for (INT i = 0; i < hp.max; i++)
		{
			if(i < hp.current)
				uHealth[i].rect = { 0,400,36,400 + 36 };
			else
				uHealth[i].rect = { 0,450,36,450 + 36 };
		}

		for (INT i = 0; i < ep.max; i++)
		{
			if (i < ep.current)
				uEnergy[i].rect = { 50,400,50 + 36,400 + 36 };
			else
				uEnergy[i].rect = { 50,450,50 + 36,460 + 36 };
		}
	}

	BOOL IsCanPlay(INT num)
	{
		if (num < 0 || num > 4)
			return FALSE;
		if (uSkill[num - 1].IsActive() == TRUE);
			uSkill[num -1].InitActive();
		return uSkill[num - 1].CanUse(*Pmana);
	}

	VOID ManagedSkillCooltime()
	{
		for (INT i = 0; i < 4; i++)
		{
			if (uSkill[i].IsUseable(*Pmana) == TRUE);
		}
	}

	BOOL IsFinishedActive(INT num)
	{
		if (num < 0 || num > 4)
			return FALSE;	
		return uSkill[num].IsActive();
		// ? 지속 시간 경과 : 발동 중 
	}
	INT GetSkillCost(INT num)
	{
		return uSkill[num - 1].GetCost();
	}
}P_UI;

class ACTOR : public OBJECT
{
private:
	TIME tOnHit;
protected:
	STATE health;
	INT moveSpeed;

public:
	ACTOR() : OBJECT()
	{
		health.max = 1;
		health.current = health.max;
		moveSpeed = 1;
		tOnHit = TIME(60);
	}
	ACTOR(INT startPosX, INT startPosY, INT width, INT height) : OBJECT( startPosX,  startPosY,  width,  height)
	{
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
		return health.current;
	}

	VOID AddHealth(INT value)
	{
		if (health.current += value < health.max)
			health.current = health.max;
	}

	VOID GetDamage(INT damage)
	{
		if((health.current -= damage) < 0)
			health.current = 0;
		tOnHit.InitOltime();
		
	}

	VOID Init(INT hp, INT mSpeed)
	{
		moveSpeed = mSpeed;
		health.max = hp;
		health.current = health.max;
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
	VOID MoveBackground()
	{
		pos.x -= GetMoveSpeed();

		if (pos.x <= SCREEN_WIDTH * -0.5f)
			pos.x = SCREEN_WIDTH * 1.5f;
	}

	INT GetMoveSpeed()
	{
		return moveSpeed;
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
	STATE energy;
	BOOL isShoted;
	INT attackType;
	TIME tFiring;
	TIME tHitDelay;
	// 스킬 관련 변수
	BOOL isFastMode = FALSE;
	BOOL isGuardMode = FALSE;
public:
	BULLET bullet[100] = { BULLET(), };
	P_UI ui = P_UI(0, 0, 1000, 180,&(energy.current));
	PLAYER(INT startPosX, INT startPosY, INT width, INT height) : ACTOR( startPosX, startPosY, width, height)
	{
		ACTOR::Init(10, 6);
		energy.max = 10;
		energy.current = energy.max;
		isShoted = FALSE;
		attackType = 0;
		tFiring = TIME(100);
		tHitDelay = TIME(666);
	}

	std::pair<STATE, STATE> GetHPnEP()
	{
		return { health , energy };
	}
	VOID GetDamage(INT damage);
	VOID Control();
	VOID OutedBorder();
	VOID ShotBullet();
	VOID UseEnergy(INT value);
	VOID PlusMoveSpeed();
	VOID ChangeAttackType();
	VOID GuardAround();
	VOID Heal();
	VOID UIManager();
};

class ENEMY : public ACTOR
{
private:
	INT type = 0;
	INT movementSize = 0; //<t> 1 = 크기, 2 = 기울기
	INT movementValue1 = 0;//<t> 1 = 폭
	INT movementValue2 = 0;//<t> 1= 높이
public:
	ENEMY() {};
	ENEMY(INT startPosX, INT startPosY, INT width, INT height) : ACTOR(startPosX, startPosY, width, height)
	{

	}
	ANIMATION moveAnim = ANIMATION();
	VOID HitWithBullet(BULLET* bullet);
	VOID HitWithPlayer(PLAYER* player);
	VOID Move();
	VOID Init(INT type);
};

VOID GameInit();
VOID GameRender();
VOID GameUpdate();
VOID GameRelease();

VOID PlayerUpdate();
VOID EnemyUpdate();

VOID AddEnemy(INT type);
template<class C1, class C2>
BOOL OnHit(C1 obj1, C2 obj2)
{
	return FALSE;
}
template<>
BOOL OnHit<BULLET,ENEMY>(BULLET obj1, ENEMY obj2)
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
BOOL OnHit<PLAYER, ENEMY>(PLAYER obj1, ENEMY obj2)
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
VOID PLAYER::GetDamage(INT damage)
{
	if (tHitDelay.IsEnoughPassed(TRUE))
	ACTOR::GetDamage(damage);
}
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
	if (KEY_DOWN(INT('Z')))
		PlusMoveSpeed();
	if (KEY_DOWN(INT('X')))
		ChangeAttackType();
	if (KEY_DOWN(INT('V')))
		Heal();
	if (KEY_DOWN(INT('A')))
		AddEnemy(1);
	if (KEY_DOWN(INT('S')))
		AddEnemy(2);
	ChangeColor();

	ui.ManagedSkillCooltime();
}
VOID PLAYER::OutedBorder()
{
	pos.x -= 2;
	if (pos.x - GetHalfWidth() < -20)
	{
		ACTOR::GetDamage(1);
		pos.x = GetHalfWidth() + 30;
	}
}
VOID PLAYER::UseEnergy(INT value)
{
	if (energy.current - value < 0)
		return;
	energy.current -= value;
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
VOID PLAYER::PlusMoveSpeed()
{
	if (ui.IsCanPlay(1) == FALSE)
		return;
	isFastMode = TRUE;
}
VOID PLAYER::ChangeAttackType()
{
	if (ui.IsCanPlay(2) == FALSE)
		return;
	++attackType %= 4;
	UseEnergy(ui.GetSkillCost(2));
}
VOID PLAYER::GuardAround() 
{
	if (ui.IsCanPlay(3) == FALSE)
		return;

}
VOID PLAYER::Heal()
{
	if ((health.current < health.max) == FALSE)
		return;
		if (ui.IsCanPlay(4))
		{
			health.current++;
			UseEnergy(ui.GetSkillCost(4));
		}
}
VOID PLAYER::UIManager() {
	ui.SetHPnEP(GetHPnEP());
	for (INT i = 0; i < 4; i++)
		if (ui.IsFinishedActive(i) == TRUE)
			switch (i)
			{
			case 1: isFastMode = FALSE;
				break;
			case 3: isGuardMode = FALSE;
				break;
			}
		else
		{

		}
}

// / ENEMY FUNCTION / //
VOID ENEMY::HitWithBullet(BULLET* bullet)
{
		if (OnHit(*bullet, *this) == FALSE)
			return;
	this->GetDamage(5);
	bullet->Outed();
}
VOID ENEMY::HitWithPlayer(PLAYER* player)
{
	if (OnHit(*player, *this) == FALSE)
		return;
	player->GetDamage(5);
}
VOID ENEMY::Move()
{

	MoveLeft();
	switch (type)
	{
	case 1:
		MoveUp(INT(movementSize / (0.1 * movementValue1) * sinf( 0.1 / movementSize * pos.x)));
		break;
	case 2:
		MoveUp(0.001 * movementSize * pos.x);
		break;
	default:return;
		break;
	}
}
VOID ENEMY::Init(INT type)
{

	this->type = type;
	switch (this->type)
	{
	case 1:
		ACTOR::Init(25, 4);
		movementSize = 0.1 * (35 + rand() % 40);
		movementValue1 = 0.1 *(90 + rand() % 30);
		movementValue2 = 0.1 * (100 - rand() % 25);
		moveAnim = ANIMATION(6, 100, this);
		break;
	case 2:
		if (pos.y <= SCREEN_HEIGHT * 0.4)
			movementSize = 1 + rand() % 2;
		else if (pos.y <= SCREEN_HEIGHT * 0.6)
			movementSize = -2 + rand() % 3;
		else
			movementSize = 1 + rand() % -4;
		ACTOR::Init(10, 8);
		moveAnim = ANIMATION(4, 70, this);
		break;
	}
}

// / CUSTOM FUNCTION / // 
#include <bangtal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define BULLETNUMBER 30
#define BALLNUMBER 10

bool gameflag = false;
bool keyflag = false;
bool armorFlag = false;
bool powerFlag = false;

int currentBulletStart = 0;
int currentBulletEnd = 0;
int bulletFrequency = 10;
int bulletTemp = 0;
int currentBall;
int stageNumber = 0;
int armorTimer = 0;
int powerTimer = 0;

struct cannonInfo {
	ObjectID object = createObject("image//cannon.png");;
	int x;
	int y = 0;
	int speed;
	int power;
};

struct ballInfo {
	ObjectID object;
	int x;
	int y;
	int size;
	int power;
	int speedX;
	double speedY;
	bool item;
	bool life;
};

struct bulletInfo {
	ObjectID object;
	int x;
	int y;
	int power;
	const int speed = 10;
	bool life;
};

struct itemInfo {
	ObjectID object;
	int x;
	int y;
	int type; //0->armor 1->X2
	double speedY;
	bool used;
	bool exist;
};

struct stageButtonInfo {
	ObjectID object;
	bool flag;
};

cannonInfo cannon;
ballInfo balls[300];
bulletInfo bullets[BULLETNUMBER];
itemInfo items[10];
stageButtonInfo stages[5];

SceneID home = createScene("home", "image//game.png");
SceneID game = createScene("game", "image//game.png");
SceneID selectScene = createScene("select", "image//game.png");

ObjectID startButton = createObject("image//start.png");
ObjectID helpButton = createObject("image//helpButton.png");
ObjectID helper = createObject("image//helper.png");
ObjectID endButton = createObject("image//end.png");
ObjectID clearImage = createObject("image//clear.png");
ObjectID failImage = createObject("image//fail.png");
ObjectID returnButton = createObject("image//return.png");

TimerID endTimer = createTimer(3.0f);
TimerID animeTimer = createTimer(0.01f);

//�������� flag�� ���� �������� ���� ��ư ���� �� ��ġ
void stageButton() {
	char ch[30];

	for (int i = 0; i < 5; i++) {
		sprintf_s(ch, "image//%d_%d.png", i + 1, stages[i].flag);
		stages[i].object = createObject(ch);
		scaleObject(stages[i].object, 2.0 / 3);
		locateObject(stages[i].object, selectScene, i * 200 + 190, 520);
		showObject(stages[i].object);
	}
}

//armor������ ���¿� ���� ������ �̹��� ����
void cannonChange(bool type) {
	hideObject(cannon.object);
	if (type) cannon.object = createObject("image//cannonArmor.png");
	else cannon.object = createObject("image//cannon.png");
	scaleObject(cannon.object, 0.1f);
	locateObject(cannon.object, game, cannon.x, cannon.y);
	showObject(cannon.object);
}

//���� ũ�⸦ ����
double ballScaleCal(ballInfo* ball) {
	return 0.06 * ((double)ball->size + 1);
}

//(ax, ay) (bx, by) �� ��ǥ ���� �Ÿ� ���
double distanceCal(int ax, int ay, int bx, int by) {
	return sqrt(pow((double)ax - bx, 2) + pow((double)ay - by, 2));
}

//���� ������ ����
int ballRad(ballInfo* ball) {
	return 500 * ballScaleCal(ball);
}

//�� �߽� x��ǥ
int ballCenterX(ballInfo* ball) {
	return ball->x + ballRad(ball);
}

//�� �߽� y��ǥ
int ballCenterY(ballInfo* ball) {
	return ball->y + ballRad(ball);
}

//��� �ϳ��� ���� stageNumber-1�� ����� ������ �Ͽ� ���̵� ����
bool difficultyCheck() {
	for (int i = 0; i < BALLNUMBER; i++) if (balls[i].size >= stageNumber - 1) return false;
	return true;
}

//���� ���� �� �� ����
void initialBallSet() {
	char ch[30];

	for (int i = 0; i < BALLNUMBER; i++) {
		balls[i].x = 300 + 10 * (rand() % 68);
		balls[i].y = 669 + 10 * (rand() % 5);
		balls[i].size = rand() % stageNumber;
		balls[i].power = pow(2, balls[i].size);
		balls[i].speedX = 2 * (rand() % 5 - 2);
		balls[i].speedY = -0.3;
		balls[i].item = false;
		balls[i].life = true;
		sprintf_s(ch, "image//ball%d.png", balls[i].size + 1);
		balls[i].object = createObject(ch);
		scaleObject(balls[i].object, ballScaleCal(balls + i));
		showObject(balls[i].object);
	}
	currentBall = BALLNUMBER;
}

//�� ��ġ��Ű��
void locateBall() {
	for (int i = 0; i < currentBall; i++)
		if (balls[i].life) locateObject(balls[i].object, game, balls[i].x, balls[i].y);
}

//�Ѿ� ��ġ��Ű��
void locateBullet() {
	for (int i = 0; i < BULLETNUMBER; i++) 
		if (bullets[i].life) locateObject(bullets[i].object, game, bullets[i].x, bullets[i].y);
}

//������ ��ġ��Ű��
void locateItem() {
	for (int i = 0; i < 2; i++) 
		if (items[i].exist) locateObject(items[i].object, game, items[i].x, items[i].y);
}

//���ο� �Ѿ� ���� (�� ����)
void fireBullet() {
	char ch[50];
	int t = currentBulletEnd++ % BULLETNUMBER;

	bullets[t].power = cannon.power;
	sprintf_s(ch, "image//bullet%d.png", bullets[t].power);
	bullets[t].object = createObject(ch);
	bullets[t].x = cannon.x + 35;
	bullets[t].y = 113;
	bullets[t].life = true;
	scaleObject(bullets[t].object, 0.05f);
	locateObject(bullets[t].object, game, bullets[t].x, bullets[t].y);
	showObject(bullets[t].object);
}

//�Ѿ˰� ���� �浹�ߴ��� �˻�
void hitBullet() {
	int randnum;
	char ch[30];

	for (int i = 0; i < currentBall; i++) {
		if (balls[i].life) {									   //���� ������� ��쿡��
			for (int j = 0; j < BULLETNUMBER; j++) {
				if (bullets[j].life &&							   //�Ѿ��� ����ְ�
					ballRad(balls + i) >=						   //���� �߽ɰ� �Ѿ˰��� �Ÿ��� ���� ���������� ���� ��
					distanceCal(bullets[j].x + 25, bullets[j].y + 37, ballCenterX(balls + i), ballCenterY(balls + i))) {

					bullets[j].life = false;
					hideObject(bullets[j].object);
					
					if (balls[i].power - bullets[j].power <= 0) {   //���� ü���� 0 ������ ���
						balls[i].life = false;
						hideObject(balls[i].object);

						//�� �� ���� ������
						if (balls[i].size > 0) {
							for (int k = 0; k < 2; k++) {
								balls[currentBall].size = balls[i].size - 1;
								balls[currentBall].x = ballCenterX(balls + i) - ballRad(balls + currentBall);
								balls[currentBall].y = ballCenterY(balls + i) - ballRad(balls + currentBall );							
								balls[currentBall].power = pow(2, balls[currentBall].size);
								balls[currentBall].speedY = -0.3;
								balls[currentBall].item = false;
								balls[currentBall].life = true;
								sprintf_s(ch, "image//ball%d.png", balls[currentBall].size + 1);
								balls[currentBall].object = createObject(ch);
								scaleObject(balls[currentBall].object, ballScaleCal(balls + currentBall));
								showObject(balls[currentBall++].object);
							}

							//�������� �� ���� ���� ����ӵ��� ��ġ���� �ʵ��� ��
							balls[currentBall - 2].speedX = 2 * (rand() % 5 - 2);
							do { randnum = 2 * (rand() % 5 - 2);
							} while (balls[currentBall - 2].speedX == randnum);
							balls[currentBall - 1].speedX = randnum;
						}

						//���� �μ��� ���� �������� �����ٸ�
						if (balls[i].item) {
							balls[i].item = false;

							//���� ������ ���� �������� �̴´�
							do { randnum = rand() % 2;
							} while (items[randnum].used);
							items[randnum].used = true;
							items[randnum].exist = true;

							//�������� �߽ɰ� ���� �߽��� ��ġ��Ų��
							items[randnum].x = ballCenterX(balls + i) - 50;
							items[randnum].y = ballCenterY(balls + i) - 50;
							items[randnum].speedY = 0;

							locateObject(items[randnum].object, game, items[randnum].x, items[randnum].y);
							showObject(items[randnum].object);
						}
					}
					else balls[i].power -= bullets[j].power;
				}
			}
		}
	}
}

//�Ѿ��� ȭ�� ������ ����� �� ����
void bulletInSky() {
	for (int i = 0; i < BULLETNUMBER; i++) {
		if (bullets[i].life) {
			if (bullets[i].y > 720) {
				bullets[i].life = false;
				hideObject(bullets[i].object);
			}
		}
	}
}

//��� ���� �μ����� ��� true�� ����
bool clearCheck() {
	for (int i = 0; i < currentBall; i++) if (balls[i].life) return false;
	return true;
}

//������ ���� �浹�� �˻�
bool failCheck() {
	if (armorFlag) return false;	//armor������ Ȱ��ȭ �����̸� �˻� ����
	for (int i = 0; i < currentBall; i++) {  
		if (balls[i].life &&		//���� ����ְ�
			(ballRad(balls + i) >=	//������ �� ���� ���� �߽ɰ��� �Ÿ� �� �ϳ��� ���� ���������� ������ true
			 distanceCal(cannon.x + 60, cannon.y + 102, ballCenterX(balls + i), ballCenterY(balls + i)) ||
			 ballRad(balls + i) >=
			 distanceCal(cannon.x + 17, cannon.y + 50, ballCenterX(balls + i), ballCenterY(balls + i)) ||
			 ballRad(balls + i) >=
			 distanceCal(cannon.x + 102, cannon.y + 50, ballCenterX(balls + i), ballCenterY(balls + i))))
			return true;
	}
	return false;
}

//�������� ���� ���� �����ϰ� �̴´�
void selectItemBall() {
	int number;

	for (int i = 0; i < 2; i++) {
		do { number = rand() % BALLNUMBER;
		} while (balls[number].item);
		balls[number].item = true;
	}
}

//������ �ʱ�ȭ
void itemMaker() {
	items[0].object = createObject("image//armor.png");
	items[1].object = createObject("image//powerup.png");
	for (int i = 0; i < 2; i++) {
		items[i].type = i;
		items[i].used = false;
		items[i].exist = false;
		scaleObject(items[i].object, 100.0 / 200);
	}	
}

//������ �������� �浹 üũ
void itemCheck() {
	for (int i = 0; i < 2; i++) {
		if (items[i].exist) {						//�������� ȭ�鿡 ������ ��
			if (items[i].y <= 68 && cannon.x + 88 >= items[i].x && cannon.x - 68 <= items[i].x) {
				hideObject(items[i].object);
				items[i].exist = false;

				if (items[i].type == 0) {			//������ armor
					armorFlag = true;
					armorTimer = 1000;
					cannonChange(true);
				}
				else if (items[i].type == 1) {      //������ X2
					cannon.power = 2;
					powerFlag = true;
					powerTimer = 1000;
				}
			}
		}
	}
}

//���� ����
void gameStart() {
	gameflag = true;
	enterScene(game);
	startTimer(animeTimer);

	currentBulletStart = 0;
	currentBulletEnd = 0;
	cannon.x = 580;
	cannon.power = 1;
	cannonChange(false);

	itemMaker();
	do { initialBallSet();
	} while (difficultyCheck());	
	selectItemBall();
}

//���� Ŭ���� ��
void clearGame() {
	showObject(clearImage);
	startTimer(endTimer);
	stages[stageNumber].flag = true;
}

//���� ���� ��
void failGame() {
	showObject(failImage);
	startTimer(endTimer);
}

//������ �������� ��
void gameEnd(bool param) {
	stopTimer(animeTimer);
	gameflag = false;
	keyflag = false;
	armorFlag = false;
	powerFlag = false;
	armorTimer = 0;
	powerTimer = 0;
	bulletTemp = 0;

	if (param) clearGame();
	else failGame();
	cannon.x = 580;
	stageNumber = 0;
}

void mouseCallBack(ObjectID object, int x, int y, MouseAction action) {
	if (object == startButton) { stageButton(); enterScene(selectScene); }
	else if (object == endButton) endGame();
	else if (object == helper) hideObject(helper);
	else if (object == helpButton) showObject(helper);
	else if (object == returnButton) enterScene(home);
	else if (object == stages[0].object && stages[0].flag) { stageNumber = 1; gameStart(); }
	else if (object == stages[1].object && stages[1].flag) { stageNumber = 2; gameStart(); }
	else if (object == stages[2].object && stages[2].flag) { stageNumber = 3; gameStart(); }
	else if (object == stages[3].object && stages[3].flag) { stageNumber = 4; gameStart(); }
	else if (object == stages[4].object && stages[4].flag) { stageNumber = 5; gameStart(); }
}

void keyboardCallBack(KeyCode code, KeyState state) {
	if (gameflag) {
		if (code == KeyCode::KEY_LEFT_ARROW && state == KeyState::KEY_PRESSED) {
			keyflag = true;
			cannon.speed = -10;
		}
		else if (code == KeyCode::KEY_RIGHT_ARROW && state == KeyState::KEY_PRESSED) {
			keyflag = true;
			cannon.speed = 10;
		}
		else if (state == KeyState::KEY_RELEASED) keyflag = false;
	}
}

void timerCallBack(TimerID timer) {
	if (timer == animeTimer) {	//0.01��
		if (failCheck()) gameEnd(false);
		else if (clearCheck()) gameEnd(true);

		//������ x��ǥ�� ����
		if (keyflag == true) {   //Ű���� ���� ������ ��
			if (cannon.x + cannon.speed >= 1160) cannon.x = 1160;
			else if (cannon.x + cannon.speed <= 0) cannon.x = 0;
			else cannon.x += cannon.speed;
			locateObject(cannon.object, game, cannon.x, cannon.y);
		}

		//���� x�� y��ǥ ����
		for (int i = 0; i < currentBall; i++) {
			if (balls[i].life) {														//���� ���������
				if (balls[i].x <= 0 || balls[i].x + 2 * ballRad(balls + i) >= 1280)		//���� ���� �΋H����
					balls[i].speedX *= -1;												//������ �ӷ����� ƨ�ܳ���
				
				balls[i].speedY += 0.05;
				if (balls[i].y - balls[i].speedY <= 0)									//�ٴڿ� ���� �΋H����
					balls[i].speedY = -1 * ((double)(rand() % 5) + 6);					//�������� Ƣ������� �ӵ� ����
				else if (balls[i].y - balls[i].speedY >= 720) balls[i].speedY = -0.3;	//�ʹ� ���� ���� �ʵ��� ����
			
				balls[i].x += balls[i].speedX;
				balls[i].y -= balls[i].speedY;
			}
		}

		//�Ѿ� y��ǥ ����
		for (int i = 0; i < BULLETNUMBER; i++) if (bullets[i].life) bullets[i].y += bullets[i].speed;

		//������ y��ǥ ����
		for (int i = 0; i < 2; i++) {
			if (items[i].exist) {											//�������� ȭ�鿡 ������ ��
				if (items[i].y - items[i].speedY <= 0) items[i].y = 0;		//�������� �ٴڿ� ������ ������
				else {														//�ƴϸ� ����߸�
					items[i].speedY += 0.05;
					items[i].y -= items[i].speedY;
				}
			}	
		}

		//�Ѿ��� ���� �ֱ⸶�� �߻��ϵ��� ��
		if (bulletTemp++ >= bulletFrequency) { fireBullet(); bulletTemp = 0; }

		//armor ������ Ȱ��ȭ �� 
		if (armorFlag) {
			armorTimer--;

			//�ð��� ���� ���������� �����Ÿ��� Ƚ�� ����
			if (armorTimer == 200 || armorTimer == 100 || armorTimer == 50 || armorTimer == 30 || armorTimer == 10) 
				cannonChange(false);
			else if (armorTimer == 150 || armorTimer == 75 || armorTimer == 40 || armorTimer == 20 || armorTimer == 5) 
				cannonChange(true);
			else if (armorTimer <= 0) {
				cannonChange(false);
				armorFlag = false;
			}
		}

		//X2 ������ Ȱ��ȭ ��
		if (powerFlag) {
			powerTimer--;
			if (powerTimer <= 0) {
				cannon.power = 1;
				powerFlag = false;
			}
		}

		locateBall();
		locateBullet();
		locateItem();
		hitBullet();
		bulletInSky();
		itemCheck();

		if (gameflag) {
			stopTimer(animeTimer);
			setTimer(animeTimer, 0.01f);
			startTimer(animeTimer);
		}
	}
	else if (timer == endTimer) {
		stopTimer(endTimer);
		setTimer(endTimer, 3.0f);
		stageButton();
		enterScene(selectScene);
		hideObject(clearImage);
		hideObject(failImage);

		for (int i = 0; i < currentBall; i++) {
			balls[i].life = false;
			hideObject(balls[i].object);
		}
		for (int i = 0; i < BULLETNUMBER; i++) {
			bullets[i].life = false;
			hideObject(bullets[i].object);
		}	
		for (int i = 0; i < 2; i++) {
			items[i].exist = false;
			items[i].used = false;
			hideObject(items[i].object);
		}
	}
}

int main() {
	srand(time(NULL));	//rand() �Լ�

	setMouseCallback(mouseCallBack);
	setTimerCallback(timerCallBack);
	setKeyboardCallback(keyboardCallBack);

	setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);
	setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);

	scaleObject(cannon.object, 0.1f);
	scaleObject(startButton, 0.1f);
	scaleObject(helpButton, 100.0 / 720);
	scaleObject(endButton, 0.09f);
	scaleObject(returnButton, 100.0 / 225);

	locateObject(startButton, home, 390, 520);
	locateObject(helpButton, home, 590, 520);
	locateObject(endButton, home, 793, 527);
	locateObject(helper, home, 200, 150);
	locateObject(returnButton, selectScene, 590, 320);
	locateObject(failImage, game, 490, 400);
	locateObject(clearImage, game, 490, 400);

	showObject(helpButton);
	showObject(startButton);
	showObject(endButton);
	showObject(returnButton);

	stages[0].flag = true;

	startGame(home);
}
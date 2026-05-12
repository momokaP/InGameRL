# InGameRL

[프로젝트 개요](https://github.com/momokaP/InGameRL/blob/main/InGameRL%20%EC%9A%94%EC%95%BD%20%ED%8F%AC%EC%8A%A4%ED%84%B0.pdf)

[자세한 설명](https://github.com/momokaP/InGameRL/blob/main/InGameRL%20%EC%84%A4%EB%AA%85.pdf)

<img width="922" height="506" alt="화면 캡처 2025-11-14 031953" src="https://github.com/user-attachments/assets/722c9523-a643-4c7d-88c8-000d052a4713" />

---

# Run



---

# 프로젝트 소개

기존 게임 AI는 개발사가 미리 설계한 행동 패턴을 플레이어에게 제공하는 방식이 대부분입니다.

이 프로젝트는 다음 질문에서 시작되었습니다.

> “플레이어가 AI 학습 과정에 직접 참여할 수 있는 게임 구조를 만들 수 있을까?”

Unreal Engine 5와 Learning Agents 플러그인에서 제공하는 PPO 기반 강화학습을 활용하여,  
플레이어 참여형 AI 학습 및 사용 시스템을 구현한 프로젝트입니다.

---

# 프로젝트 목표

- 플레이어 참여형 AI 학습 구조 구현
- 물리 기반 전투 환경에서 강화학습 적용
- 게임 실행 환경과 학습 환경의 분리
- 실시간 학습 가능한 구조 설계

---

# Tech Stack

- Unreal Engine 5
- Learning Agents
- PPO (Proximal Policy Optimization)
- C++
- Python

---

# System Architecture

<img width="1696" height="868" alt="시스템구조" src="https://github.com/user-attachments/assets/6ba410f1-8e29-4847-895c-ce540625129b" />

게임 실행 프로세스와 강화학습 프로세스를 하나의 구조로 구성할 경우:

- 파이썬에서 학습을 하는 동안 언리얼 엔진 중단 발생
- 학습과 게임 구조의 결합도 증가
- 디버깅 및 확장 어려움

이를 해결하기 위해:

- 게임 프로세스
- 학습 프로세스

를 분리하고 Socket 기반으로 통신하도록 설계하였습니다.

---

# Reinforcement Learning Environment

## Observation

에이전트가 전투 상황을 인식하기 위해 다음 정보를 사용했습니다.

- 상대 위치 및 방향
- 캐릭터 위치 및 방향
- 양손 위치 및 방향

## Action

에이전트는 다음 행동을 수행할 수 있습니다.

- 캐릭터 이동
- 캐릭터 회전
- 양손 위치 이동
- 양손 방향 회전

## Reward

초기 Reward 구성:

- 적과의 거리
- 공격 성공 여부
- 생존 상태

추가 개선 요소:

- Facing Reward (상대 정면 유지)
- Gaussian Distance Reward (적절한 거리 유지)
- Stamina Penalty (과도한 행동 제한)

---

# Problem Solving

## Problem

초기 학습 과정에서 에이전트가:

- 서로 밀착하며 회전
- 팔을 과도하게 휘두름

하는 문제가 발생하였습니다.

---

## Cause Analysis

원인 분석 결과:

- 단순 거리 기반 reward 구조
- 방향 유지 reward 부재
- 손 이동에 대한 패널티 부족

문제로 인해 local optimum에 수렴하고 있음을 확인하였습니다.

---

## Solution

### 1. Facing Reward 추가

논문:

> Control Strategies for Physically Simulated Characters Performing Two-player Competitive Sports

를 참고하여 방향 유사도 기반 reward를 추가하였습니다.

이를 통해 에이전트가 서로를 바라보도록 유도하였습니다.

---

### 2. Gaussian Distance Reward 적용

기존 distance reward는 가까워질수록 reward가 증가하여 과도한 밀착이 발생했습니다.

이를 해결하기 위해 특정 거리에서 최대 reward를 가지는 Gaussian 기반 reward 함수로 변경하였습니다.

이를 통해 일정 거리 유지와 과도한 밀착 감소를 유도하였습니다.

### 3. Stamina System 도입

양손 이동 시 stamina가 증가하도록 설계하고,

stamina가 높을수록 reward를 감소시키도록 수정하였습니다.

이를 통해 과도한 손 휘두르기 행동을 감소시켰습니다.

---

# Result

Reward 구조 개선 전

Reward 구조 개선 후

Reward 구조 개선 이후:

- 에이전트 간 비정상적인 밀착 감소
- 정면 방향 유지
- 불필요한 손 휘두르기 감소
- 보다 안정적인 전투 행동 형성

을 확인할 수 있었습니다.

또한:

- 플레이어 참여형 AI 학습 구조 구현
- 실시간 학습 및 플레이 연동 구조 구현
- 게임 프로세스와 학습 프로세스 분리 구조 구현

을 완료하였습니다.

---

# Limitations

프로젝트를 진행하며 다음과 같은 한계를 확인하였습니다.

- 높은 학습 연산 비용
- 실시간 학습 속도 문제
- 높은 행동 자유도로 인한 학습 수렴 난이도 증가

특히 실시간 학습은 가능했지만, 의미 있는 학습 결과까지 필요한 시간이 길어 실제 상용 게임 적용에는 한계가 있음을 확인하였습니다.

---

# Future Improvements

향후에는:

- GPU 기반 외부 학습 서버 분리
- 행동 자유도 최적화
- 학습 구조 경량화

등을 통해 성능을 개선하고자 합니다.

---

# What I Learned

본 프로젝트를 통해:

- 강화학습 환경 설계
- Reward Engineering
- 물리 기반 시뮬레이션
- 프로세스 분리 구조 설계
- 논문 기반 문제 해결 과정

을 경험할 수 있었습니다.

특히 단순 구현보다:

"왜 이런 문제가 발생하는가?"

를 분석하고 구조적으로 개선하는 과정의 중요성을 배울 수 있었습니다.


# InGameRL

[프로젝트 개요](https://github.com/momokaP/InGameRL/blob/main/InGameRL%20%EC%9A%94%EC%95%BD%20%ED%8F%AC%EC%8A%A4%ED%84%B0.pdf)

[자세한 설명](https://github.com/momokaP/InGameRL/blob/main/InGameRL%20%EC%84%A4%EB%AA%85.pdf)

<img width="922" height="506" alt="화면 캡처 2025-11-14 031953" src="https://github.com/user-attachments/assets/722c9523-a643-4c7d-88c8-000d052a4713" />


### 전투 시연 영상

https://github.com/user-attachments/assets/af6cfbfc-b6f8-4f0d-87d4-8e24ef8fdc17

### 학습 시연 영상

https://github.com/user-attachments/assets/b8d129af-52a5-4997-9b64-bc99dc159288

---

# Run

---

# 프로젝트 소개

기존 게임 AI는 개발사가 미리 설계한 행동 패턴을 플레이어에게 제공하는 방식이 대부분입니다.

이 프로젝트는 다음 질문에서 시작되었습니다.

> “플레이어가 AI 학습 과정에 직접 참여할 수 있는 게임 구조를 만들 수 있을까?”

Unreal Engine 5와 Learning Agents 플러그인에서 제공하는 PPO 기반 강화학습을 활용하여,

플레이어가 직접 AI의 전투 성향을 학습시키고,
학습된 AI를 실제 전투에 사용할 수 있는
플레이어 참여형 AI 학습 및 사용 시스템을 구현한 프로젝트입니다.

사용자는 reward weight를 조정하여:

- 공격적인 AI
- 방어적인 AI

등 원하는 전투 성향으로 AI를 학습시킬 수 있으며,

학습이 완료된 AI를 선택하여 실제 전투에 반영할 수 있도록 설계하였습니다.

---

# 프로젝트 목표

- 플레이어 참여형 AI 학습 구조 구현
- Reward Weight 기반 사용자 맞춤형 AI 학습 시스템 구현
- 학습된 AI 저장 및 선택 시스템 구현
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

# User-driven AI Training

본 프로젝트에서는 사용자가 직접 AI의 전투 성향을 학습시킬 수 있도록 설계하였습니다.

기본적으로 기초 행동이 사전 학습된 초기 전투 AI 네트워크를 제공하며,
사용자는 reward weight를 조정하여 AI의 행동 성향을 변화시킬 수 있습니다.

예를 들어:

- 적의 체력 상황에 대한 reward weight를 높일 경우
  → 공격적인 전투 성향 학습

- 자신의 체력 상황에 대한 reward weight를 높일 경우
  → 생존 중심의 방어적인 전투 성향 학습

과 같은 방식으로 AI를 사용자 맞춤형으로 학습시킬 수 있습니다.

학습이 완료된 AI는 저장되며,
플레이어는 전투 시작 전 원하는 AI를 선택하여 실제 전투에 사용할 수 있습니다.

이를 통해:

- 플레이어 참여형 AI 성장 구조
- 사용자 맞춤형 전투 스타일
- 학습 결과의 실제 게임 반영

을 구현하고자 하였습니다.

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

Reward 구조 개선 이후 다음과 같은 변화를 확인할 수 있었습니다.

### 학습 개선에 성공한 부분

- 상대에게 접근
- 일정 거리 유지
- 상대 정면 바라보기
- 과도한 밀착 감소
- 불필요한 손 움직임 감소

이를 통해 에이전트가 기본적인 전투 positioning 행동은 어느 정도 학습할 수 있음을 확인하였습니다.

또한 본 프로젝트의 목표인

- Reward Weight 기반 사용자 맞춤형 AI 학습 구조 구현
- 학습된 AI 저장 및 선택 시스템 구현
- 실시간 학습 및 플레이 연동 구조 구현
- 게임 프로세스와 학습 프로세스 분리 구조 구현

을 완료하였습니다.

---

### 한계가 남은 부분

반면 실제 전투 행동은 여전히 제한적인 수준에 머물렀습니다.

특히 양손 움직임의 경우:

- 천천히 움직이며
- 공격 의도가 드러나지 않고
- 의미 없는 궤적으로 반복 이동하는

행동 패턴이 자주 발생하였습니다.

즉:

- 이동 및 방향 제어는 어느 정도 학습되었지만,
- 실제 공격 전략 및 무기 활용 행동은 충분히 학습할 수 있는 수준은 되지 못했습니다.

---

# Limitations

프로젝트를 진행하며 다음과 같은 한계를 확인하였습니다.

- 높은 학습 연산 비용
- 실시간 학습 속도 문제
- 높은 행동 자유도로 인한 학습 수렴 난이도 증가

특히 전투 학습에서는:

- 이동
- 방향 제어
- 양손 위치 제어
- 양손 회전
- 무기 충돌

등을 동시에 학습해야 했기 때문에 action space가 복잡해지는 문제가 있었습니다.

그 결과 에이전트는:

- 상대에게 접근
- 정면 방향 유지

등의 기본 행동은 학습할 수 있었지만,

실제 전투에서는:

- 비효율적인 팔 움직임
- 느리고 의미 없는 공격 궤적
- 불안정한 공격 패턴

등의 문제가 발생하였습니다.

특히 현재 reward 구조만으로는:

- 공격 의도
- 효과적인 검 궤적
- 타이밍 기반 공격 전략

등을 충분히 유도하기 어려웠습니다.

이를 통해 물리 기반 전투에서는
모든 행동에 자유도를 부여하여 학습시키기보다,

- 이동 및 전략은 강화학습
- 실제 공격 동작은 애니메이션 기반 시스템

으로 분리하는 hybrid 구조가 현실적으로 더 적합할 수 있다는 점을 생각해보게 되었습니다.

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


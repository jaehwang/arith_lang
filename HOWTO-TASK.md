# HOWTO: AI 도구 협업으로 대규모 작업(Task) 수행하기

이 문서의 목적은 AI를 이용한 개발에서 큰 Task를 Sub Task로 나눠 진행하기 위한 효과적인 방법론을 제시하는 것입니다. [`specs/variables.md`](specs/variables.md), [`plans/mut-implementation-plan.md`](plans/mut-implementation-plan.md) 등의 파일을 사례로 이 방법론을 소개합니다. Sub Task 간의 '의도 이탈(drift)' 문제를 해결하기 위해 명세(spec)를 '단일한 진실의 원천(Single Source of Truth)'으로 삼고 각 작업 단계마다 이를 기준으로 일관성을 유지하는 것이 핵심입니다.

## 1) 배경: Context Engineering의 필요성과 '의도 이탈' 문제

> AI Coding의 가장 큰 도전 과제:
>
> Keeping agents on track through long, complex tasks. 
>
> [Cline](https://www.linkedin.com/posts/clinebot_cline-v325-is-live-addressing-the-biggest-ugcPost-7362148463586783232-NT_x?utm_source=share&utm_medium=member_desktop&rcm=ACoAAAEzDxcBAW1X5QEW8rSPInqKRgAXWRGOYfk)

AI 도구를 활용한 협업 개발에서는 각 도구가 작업 맥락(context)을 어떻게 해석하는지에 따라 결과가 달라질 수 있습니다. 여러 도구와 단계가 얽힌 대규모 작업에서는, 맥락 전달이 불충분하거나 일관되지 않으면 초기 의도에서 벗어나는 '의도 이탈(drift)' 문제가 빈번하게 발생합니다. 


## 2) 핵심 원칙

1.  **단일 진실의 원천(SoT)**: 명세 파일(e.g. specs/variables.md)을 최우선 참조한다.
2.  **Sub Task**: 명세를 구현하기 위한 Sub Task를 바탕으로 계획을 작성한다.
3.  **단계 경계 = 커밋 경계**: 각 sub task가 끝날 때마다 커밋한다. 커밋 메시지에는 해당 작업이 참조한 spec, 계획 단계, 테스트 범위를 명확히 기록한다.
5.  **지속적 드리프트 감지**: 테스트 케이스는 사람이 직접 작성하거나 AI가 생성할 수 있다. 생성된 테스트 케이스가 명세(spec)에 부합하는지 반드시 사람이 리뷰하여 드리프트를 감지한다.
6.  **도구 및 Sub Task 전환 시 일관성 유지**: 도구 간 또는 Sub Task 간 전환에서는 AGENTS.md, rules/anchor-comments.md, rules/commit-log.md 등 프로젝트 규칙을 통해 일관성을 유지한다.
8.  **명시적 범위 관리**: 핵심 기능에 집중하고, 의존성이 있거나 후순위인 기능은 spec 문서의 "Implementation Status"나 "로드맵"에 명시하여 작업을 분리한다.


## 3) 전체 워크플로 개요

* 준비
  1. 프로젝트 규칙 준수: AGENTS.md, rules/anchor-comments.md, rules/commit-log.md, memory-bank/ 등 AI가 준수해야 하는 규칙을 명확히 한다. 
  2. SoT 지정: specs/variables.md을 작성한다. AI를 이용해 작성한 후 리뷰하는 방식을 권장한다. 요구 사항, 설계, 테스트 케이스 등을 포함한 명세 파일을 단일한 진실의 원천으로 삼는다.
  3. 계획 수립
     - `plans/mut-implementation-plan.md`. Sub Task들을 도출하고 구현 순서를 작성한다. 이 문서 역시 AI로 작성하고 사람이 리뷰하는 방식을 권장한다.
     - 계획을 만들 때 사용할 수 있는 프롬프트로 Cline의 [Deep Planning](https://github.com/cline/cline/blob/main/docs/features/slash-commands/deep-planning.mdx)을 참고할 수 있다.
* 실행 루프(각 단계 반복)
  1. Sub Task 구현/리팩토링
  2. 테스트 케이스 작성 (필요시)
  3. 자동화 실행(빌드, 테스트)
  4. 결과 검토 및 드리프트 감지/교정
  5. 커밋

* 종료
  - 회귀 테스트/린팅/포맷/문서화
  - 변경 로그/릴리스 노트에 spec 준수 항목 요약
  - specs/variables.md의 Implementation Status로 현재 범위 명시, 로드맵 항목 분리


## 4) 실전 사례

다음 두 파일은 모두 사용자의 간단한 입력을 바탕으로 AI가 생성한 산출물입니다. 이들은 명세(specs/variables.md)와 구현 계획(plans/mut-implementation-plan.md)을 기반으로 하여, AI 도구가 어떻게 협업하여 대규모 작업을 수행할 수 있는지를 보여줍니다.

### 4.1) [variables.md](specs/variables.md) (명세)
  - 요구 사항
    - 기본 불변(immutable-by-default), `mut`로 명시적 가변 선언
    - 섀도잉 허용, 재할당은 타입 일치 강제
    - 정밀 진단(file:line:col, 소스 라인, 캐럿, note/help)과 미정의 변수 에러
  - 설계
    - Syntax: 선언/할당 BNF와 예제
    - Semantics: 불변/가변/섀도잉/스코프 규칙, 섀도잉 vs 뮤테이션 차이
    - 구현 가이드 핵심: Symbol(name, type, is_mutable, is_initialized), AssignmentExprAST 플래그, mut 파싱, 재할당 타입 검사/에러 형식
  - 테스트
    - 변수 불변성, 가변성, 섀도잉, 함수 파라미터, 스코프 등 주요 케이스별 테스트 예시와 기대 결과 포함
    - 에러 상황(재할당, 타입 불일치, 미정의 변수)에 대한 진단 메시지 형식 명확히 제시
    - 테스트 케이스는 명세의 요구 사항과 설계에 기반하여 작성됨

### 4.2) [mut-implementation-plan.md](plans/mut-implementation-plan.md) (구현 계획)
  - 목적: 명세의 요구 사항을 충실히 구현하기 위한 변경 범위와 구체적 순서 정리
  - 변경 대상: 각 컴포넌트별로 mutability(가변성) 지원 추가 및 구조 변경 제안
  - 테스트 전략: 단위/통합 테스트 파일 구조와 주요 시나리오 명확히 제시
  - 구현 순서 요약:
    1. 변수 선언/할당 로직에 mut 플래그 추가
    2. Symbol 구조체에 is_mutable, is_initialized 필드 반영
    3. AssignmentExprAST에 mut 처리 및 타입 검사 로직 구현
    4. 섀도잉/재할당 규칙 적용 및 에러 메시지 형식 통일
    5. 테스트 케이스 작성 및 명세와 일치 여부 검증

## 5) Github Copilot으로 Deep Planning 하기

Cline의 [Deep Planning 명령](https://github.com/cline/cline/blob/main/docs/features/slash-commands/deep-planning.mdx)을 Github Copilot의 사용자 Prompt로 활용할 수 있습니다.  

[`specs/functions.md`](specs/functions.md) 명세를 바탕으로 Deep Planning을 요청하는 예시입니다.

## 📝 단계별 과정

### 1단계: Prompt 추가

[`.github/prompts/deep-planning.prompt.md`](.github/prompts/deep-planning.prompt.md) 생성한다.

### 2단계: Deep Planning 요청

Agent 모드에서 다음과 같이 입력한다.

```
/deep-planning specs/functions.md 문서를 바탕으로 구현 계획을 만들어 줘.
```

[`plans/function-implementation-plan.md`](plans/function-implementation-plan.md)이 생성된다.

### 4단계: 구현 순서에 따라 실행

[`plans/function-implementation-plan.md`](plans/sample-function-implementation-plan.md)에 정의된 구현 순서에 따라 각 Sub Task를 실행한다.

## 6) 참고

* [Cline의 Deep Planning](https://docs.cline.bot/features/slash-commands/deep-planning)


## 부록: AI의 장기 작업을 위한 5계층 컨텍스트 관리 시스템

이 부록은 AI 에이전트의 '의도 이탈' 문제를 체계적으로 해결하기 위한 심층적인 프레임워크를 제안합니다.

### A.1) 핵심 개념: 5계층 컨텍스트 프레임워크

의도 이탈을 효과적으로 방지하기 위해, 우리는 5개의 독립적이지만 상호 연결된 컨텍스트 계층으로 구성된 프레임워크를 활용합니다. 각 계층은 작업의 다른 측면을 책임지며, 전체적인 일관성을 유지합니다.

*   **1. 전략 계층 (Memory Bank)**: **왜(Why)** 이 프로젝트가 존재하는가? 프로젝트의 장기적인 목표, 상태, 아키텍처 패턴을 정의합니다. 세션을 넘나드는 영구적인 기억의 역할을 합니다.
*   **2. 명세 계층 (`specs/`)**: **무엇을(What)** 만들어야 하는가? 기능의 요구사항, 설계, 테스트 케이스를 상세히 기술하는 '단일 진실의 원천(SoT)'입니다.
*   **3. 계획 계층 (`plans/`)**: **어떻게(How)** 만들 것인가? 명세를 구현 가능한 Sub Task로 나누고, 작업 순서와 범위를 정의하는 실행 계획입니다. 계획은 명세의 하위 개념이며, 명세가 변경되면 계획도 변경될 수 있습니다.
*   **4. 구현 계층 (Source Code + Tests)**: **실제 구현(Implementation)**. 명세와 계획에 따라 작성된 소스 코드와, 해당 코드가 명세에 부합하는지 검증하는 테스트 코드로 구성됩니다. 테스트는 구현이 의도에서 벗어나지 않았는지 확인하는 핵심적인 장치입니다.
*   **5. 이력 계층 (Commit Logs)**: **변경 기록(History)**. 코드 변경 사항을 명세 및 계획과 연결하여 기록하는 감사 가능한 역사입니다. "무엇을 왜 바꿨는지"를 명확히 하여 추적성을 확보합니다.

### A.2) 실전 워크플로: 5계층 모델 활용법

AI 에이전트가 복잡한 작업을 수행할 때 따라야 할 구체적인 절차는 다음과 같습니다.

*   **1단계: 세션 시작 (전략 파악)**
    1.  `AGENTS.md`를 읽고 해당 프로젝트의 규칙을 숙지합니다.
    2.  **전략 계층**: `memory-bank/` 전체를 검토하여 프로젝트의 큰 그림과 현재 상태를 파악합니다.

*   **2단계: 작업 착수 (명세와 계획 수립)**
    1.  **명세 계층**: `activeContext.md`를 바탕으로 작업할 기능의 `specs/*.md`를 작성하거나 검토합니다. (AI 초안 작성 후 리뷰 권장)
    2.  **계획 계층**: 확정된 명세를 바탕으로 `plans/*.md` 구현 계획을 수립합니다. Sub Task를 정의하고 작업 순서를 정합니다.

*   **3단계: Sub Task 실행 (구현 및 검증)**
    1.  **구현 계층**: 계획에 따라 소스 코드를 작성합니다.
    2.  명세에 정의된 테스트 케이스를 기반으로 테스트를 작성하고 실행하여, 구현이 의도에서 벗어나지 않았는지 검증합니다.

*   **4단계: 작업 완료 (이력 기록 및 동기화)**
    1.  **이력 계층**: `rules/commit-log.md` 규칙에 따라, 변경 사항이 어떤 명세와 계획에 기반했는지 명시하여 **커밋**합니다.
    2.  **전략 계층**: `progress.md`와 `activeContext.md`를 업데이트하여 프로젝트의 진행 상황을 **메모리 뱅크**에 최종적으로 동기화합니다.

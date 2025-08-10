# AI 개발 시 테스트 전략 가이드

## 개요

AI와 함께 코드를 개발할 때는 전통적인 개발과 다른 접근법이 필요합니다. AI는 빠른 코드 생성이 가능하지만, 생성된 코드의 정확성과 안정성을 보장하기 위해서는 체계적인 테스트 전략이 필수적입니다.

## AI 개발의 특성과 테스트 필요성

### AI 개발의 장점
- **빠른 프로토타이핑**: 기본 구조와 로직을 신속하게 생성
- **패턴 인식**: 기존 코드 패턴을 학습하여 일관된 코드 스타일 적용
- **광범위한 지식**: 다양한 프레임워크와 라이브러리 활용 가능

### AI 개발의 위험요소
- **논리적 오류**: 문법적으로는 올바르지만 의도와 다른 동작
- **엣지 케이스 누락**: 일반적인 케이스에만 집중하고 예외 상황 간과
- **테스트 부재**: 기능 구현에만 집중하여 검증 과정 생략

## 실제 적용 사례: ArithLang 프로젝트

### 문제 상황
- 초기에는 정상 케이스만 통과하는 파서
- 잘못된 구문(`x = 1 y = 2`)도 허용하는 문제
- 91개 중 85개만 통과하는 negative 테스트

### 해결 과정
1. **체계적 negative 테스트 구축**
   - 12개 테스트 스위트, 91개 테스트 케이스
   - 구문 오류, 타입 오류, 제어 흐름 오류 등 모든 시나리오 커버

2. **타입 체킹 시스템 도입**
   - 파싱과 코드 생성 사이에 타입 검증 단계 추가
   - 문자열-숫자 연산 혼합 등 타입 오류 검출

3. **100% 테스트 성공 달성**
   - 모든 91개 negative 테스트 통과
   - 완전한 구문 및 타입 검증 시스템 구축

### 교훈
- **AI는 positive 케이스에 특화**: negative 테스트를 의도적으로 강화해야 함
- **점진적 개선**: 실패 테스트를 하나씩 해결해 나가는 과정
- **체계적 접근**: 무작위 테스트가 아닌 문법 규칙 기반의 체계적 테스트

## 포괄적 테스트 전략

### 1. 테스트 주도 개발 (TDD) 적응

AI 개발에서는 **Test-First** 접근법이 더욱 중요합니다:

```
1. 테스트 케이스 작성 → 2. AI로 구현 → 3. 테스트 실행 → 4. 리팩토링
```

**실제 예시 (ArithLang 프로젝트)**:
```cpp
// 1. 먼저 테스트 작성
TEST(ParserTest, ParseSimpleAssignment) {
    Parser parser("x = 42;");
    auto ast = parser.parseProgram();
    EXPECT_NE(ast, nullptr);
    // 더 구체적인 검증...
}

// 2. AI에게 파서 구현 요청
// 3. 테스트 실행 및 검증
```

### 2. Positive & Negative 테스트 균형

AI는 **정상 케이스(Happy Path)**에만 집중하는 경향이 있으므로, 의도적으로 **오류 케이스**를 설계해야 합니다.

#### Positive 테스트 (정상 케이스)
- 기본 기능 동작
- 경계 조건 (최소/최대값)
- 복합 시나리오

#### Negative 테스트 (오류 케이스) 
- 구문 오류
- 타입 오류  
- 범위 오류
- 빈 입력

**ArithLang 사례**: 91개 negative 테스트 케이스로 100% 커버리지 달성
```cpp
// 구문 오류 테스트
TEST(ParserNegativeTest, MissingSemicolon) {
    EXPECT_THROW(parser.parseProgram("x = 1"), std::runtime_error);
}

// 타입 오류 테스트  
TEST(ParserNegativeTest, StringInArithmetic) {
    EXPECT_THROW(parser.parseProgram("x = \"hello\" + 1;"), std::runtime_error);
}
```

### 3. 다층 테스트 구조

#### 단위 테스트 (Unit Tests)
각 컴포넌트를 독립적으로 테스트:
- **Lexer**: 토큰화 정확성
- **Parser**: AST 생성 검증  
- **CodeGen**: LLVM IR 생성 확인

#### 통합 테스트 (Integration Tests)
전체 파이프라인 동작 검증:
```bash
# ArithLang의 통합 테스트 예시
./test_runner.sh  # 실제 .k 파일 컴파일 및 실행
```

#### 시스템 테스트 (End-to-End Tests)
실제 사용자 시나리오 검증

## AI 특화 테스트 기법

### 1. 예외 처리 집중 검증

AI가 종종 놓치는 오류 처리를 철저히 테스트:

```cpp
// AI가 구현한 함수에 대한 예외 테스트
TEST(LexerTest, InvalidNumberFormat) {
    Lexer lexer("12.34.56");  // 잘못된 숫자 형식
    EXPECT_THROW(lexer.nextToken(), std::runtime_error);
}

TEST(LexerTest, NumberEndingWithDot) {
    Lexer lexer("123.");
    EXPECT_THROW(lexer.nextToken(), std::runtime_error);
}
```

### 2. 경계값 테스트 강화

```cpp
// 빈 입력
TEST(ParserTest, EmptyProgram) {
    Parser parser("");
    auto ast = parser.parseProgram();
    EXPECT_NE(ast, nullptr);
    EXPECT_EQ(ast->statements.size(), 0);
}

// 최대 복잡도
TEST(ParserTest, DeeplyNestedExpression) {
    Parser parser("x = (((((1 + 2) * 3) - 4) / 5) + 6);");
    EXPECT_NO_THROW(parser.parseProgram());
}
```

### 3. 회귀 테스트 (Regression Tests)

AI 수정 후 기존 기능이 깨지는 것을 방지:

```cpp
// ArithLang에서 발견된 버그를 방지하는 회귀 테스트
TEST(RegressionTest, SemicolonRequiredAfterStatement) {
    // 이전에는 "x = 1 y = 2"가 잘못 파싱됨
    EXPECT_THROW(parser.parseProgram("x = 1 y = 2"), std::runtime_error);
}
```

## 테스트 자동화 전략

### 1. 지속적 검증 파이프라인

```bash
# 빌드 및 모든 테스트 실행
mkdir -p build && cd build && cmake .. && make

# 단위 테스트
cd build && ctest

# 통합 테스트  
cd .. && ./test_runner.sh
```

### 2. 테스트 주도 리팩토링

AI가 코드를 수정할 때마다 전체 테스트 스위트 실행:

```bash
# 수정 후 즉시 검증
cmake --build build && cd build && ctest && cd .. && ./test_runner.sh
```

## AI와 협업할 때의 테스트 전략

### 1. AI에게 명확한 테스트 요구사항 제시

**Good**: 
> "parseExpression 함수를 구현해주세요. 다음 negative 테스트가 모두 통과해야 합니다:
> - 빈 입력에 대해 예외 발생
> - 닫히지 않은 괄호에 대해 예외 발생  
> - 잘못된 연산자 순서에 대해 예외 발생"

**Bad**:
> "parseExpression 함수를 구현해주세요"

### 2. 테스트 결과로 AI 피드백 제공

```
AI가 구현한 코드를 테스트해보니 다음 케이스에서 실패합니다:
- TEST_F(ParserTest, InvalidNumber): "12.34.56" 입력 시 예외가 발생해야 하는데 통과함
- 예상 동작: std::runtime_error 예외 발생
- 실제 동작: 12.34로 파싱되어 성공

수정 방향: readNumber() 함수에서 여러 개의 소수점 검출 로직 추가 필요
```

### 3. 점진적 복잡도 증가

1. **기본 기능** → 단순한 positive 테스트
2. **엣지 케이스** → 경계값 테스트  
3. **오류 처리** → comprehensive negative 테스트
4. **통합** → end-to-end 테스트

## 테스트 도구 활용

### Google Test 활용
```cpp
// 파라미터화된 테스트로 효율성 증대
class InvalidSyntaxTest : public ::testing::TestWithParam<std::string> {};

INSTANTIATE_TEST_SUITE_P(
    SyntaxErrors, InvalidSyntaxTest,
    ::testing::Values(
        "x = 1 y = 2",      // 세미콜론 누락
        "x = ;",            // 값 누락
        "= 42;",            // 변수명 누락
        "x = 1 +;",         // 불완전한 연산
        "if (x > 1",        // 닫히지 않은 괄호
        "\"str\" + 1"       // 타입 오류
    )
);

TEST_P(InvalidSyntaxTest, ThrowsParseError) {
    Parser parser(GetParam());
    EXPECT_THROW(parser.parseProgram(), std::runtime_error);
}
```

### 커버리지 도구 활용
```bash
# 코드 커버리지 측정
gcov build/*.o
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## 베스트 프랙티스

### 1. Test-Driven AI Development
- 테스트 케이스를 먼저 작성하고 AI에게 구현 요청
- 실패하는 테스트부터 시작해서 점진적 개선

### 2. 포괄적 Negative Testing  
- AI는 happy path에 집중하므로 의도적으로 error path 강화
- 모든 예외 상황에 대한 테스트 케이스 작성

### 3. 지속적 검증
- AI 수정 후 즉시 전체 테스트 스위트 실행
- 회귀 버그 방지를 위한 자동화된 테스트 파이프라인

### 4. 명확한 요구사항 전달
- AI에게 구체적이고 측정 가능한 테스트 기준 제시
- 성공/실패 조건을 명확히 정의

### 5. 점진적 복잡도 관리
- 간단한 케이스부터 시작해서 복잡한 시나리오로 확장
- 각 단계에서 완전한 테스트 커버리지 확보

## 결론

AI와의 협업에서 테스트는 단순한 품질 보증을 넘어 **개발 프로세스의 핵심**입니다. 체계적인 테스트 전략을 통해 AI의 빠른 구현 능력과 인간의 논리적 검증을 결합하면, 높은 품질의 소프트웨어를 효율적으로 개발할 수 있습니다.

ArithLang 프로젝트에서 보듯이, 91개의 negative 테스트를 통한 100% 커버리지 달성은 AI 개발에서 철저한 테스트의 중요성을 보여주는 실증적 사례입니다. 

**핵심은 AI를 믿되 검증하는 것(Trust but Verify)**입니다.

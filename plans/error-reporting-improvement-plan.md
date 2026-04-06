# ArithLang 향상된 오류 진단 시스템 구현 계획

## 개요

이 계획은 ArithLang의 오류 진단 시스템을 개선하여 개발자 경험을 크게 향상시키고, IDE 통합 가능성을 열며, 초보자에게도 친화적인 오류 메시지를 제공합니다. 현재 기본적인 `파일:줄:열` 형식의 오류 위치 보고는 완료되어 있으므로, 이를 기반으로 더 발전된 기능들을 추가합니다.

**목표:**
- 다중 오류 보고: 첫 번째 오류에서 멈추지 않고 최대한 많은 오류를 한 번에 보고
- 오류 복구 메커니즘 강화: 파싱 실패 후에도 계속 진행하여 추가 오류 발견
- 컬러 출력: 터미널에서 가독성 향상을 위한 ANSI 색상 코드 지원
- 상세한 힌트 및 제안: 흔한 실수에 대한 자동 수정 제안
- LSP (Language Server Protocol) 기반 구조: IDE 통합을 위한 준비
- 오류 심각도 구분: Error, Warning, Info, Hint 레벨 지원

## 현재 상태 분석

### 기존 오류 보고 시스템
```cpp
// include/error.h
struct SourceLocation {
    std::string filename;
    size_t line;
    size_t column;
};

class ParseError {
private:
    std::string message;
    SourceLocation location;
public:
    ParseError(const std::string& msg, const SourceLocation& loc);
    std::string format() const; // Returns "file:line:col: error: message"
};
```

### 현재 한계점
1. **단일 오류 보고**: 첫 번째 오류에서 즉시 종료 (`throw ParseError(...)`)
2. **컨텍스트 부족**: 오류 위치 주변 코드를 보여주지 않음
3. **단순 메시지**: 수정 방법이나 힌트를 제공하지 않음
4. **흑백 출력**: 터미널에서 가독성이 떨어짐
5. **IDE 통합 불가**: 구조화된 오류 정보 부족

## 구현 범위

### 1. 다중 오류 수집 시스템 (Priority: 최고)
- 오류를 즉시 throw하지 않고 수집
- 파싱/타입 체킹 완료 후 모든 오류를 한 번에 출력
- 최대 오류 개수 제한 (기본값: 10개)

### 2. 오류 복구 메커니즘 (Priority: 높음)
- Synchronization points 정의 (`;`, `}`, `EOF` 등)
- 오류 발생 시 다음 안전한 지점까지 건너뛰기
- 부분적인 AST 생성으로 추가 분석 가능

### 3. 풍부한 오류 표시 (Priority: 높음)
- 오류 위치 주변 소스 코드 표시 (전후 2줄)
- 캐럿(^) 또는 물결(~)로 정확한 위치/범위 표시
- 다중 위치 하이라이팅 (관련된 여러 위치 동시 표시)

### 4. 컬러 출력 시스템 (Priority: 중간)
- ANSI 색상 코드 지원
- 오류/경고/정보별 색상 구분
- `--no-color` 플래그로 비활성화 가능
- CI/CD 환경 자동 감지

### 5. 스마트 제안 시스템 (Priority: 중간)
- 흔한 오류 패턴 인식 및 수정 제안
- 철자 오류 감지 (Levenshtein distance)
- Did you mean? 제안
- 구문 오류에 대한 수정 예시

### 6. LSP 준비 구조 (Priority: 낮음)
- 구조화된 진단 정보 (Diagnostic 클래스)
- JSON 직렬화 지원
- 범위(Range) 기반 오류 표시
- 수정 제안(CodeAction) 데이터 구조

## 타입 및 자료구조

### 진단 심각도 레벨
```cpp
enum class DiagnosticSeverity {
    Error,      // 컴파일 실패
    Warning,    // 경고 (컴파일은 성공)
    Info,       // 정보성 메시지
    Hint        // 개선 제안
};
```

### 소스 범위 표현
```cpp
struct SourcePosition {
    size_t line;
    size_t column;
    
    SourcePosition(size_t l, size_t c) : line(l), column(c) {}
    
    bool operator<(const SourcePosition& other) const {
        return line < other.line || (line == other.line && column < other.column);
    }
};

struct SourceRange {
    std::string filename;
    SourcePosition start;
    SourcePosition end;
    
    SourceRange(const std::string& file, SourcePosition s, SourcePosition e)
        : filename(file), start(s), end(e) {}
    
    // 단일 위치 범위
    static SourceRange fromLocation(const SourceLocation& loc) {
        return SourceRange(loc.filename, 
                          SourcePosition(loc.line, loc.column),
                          SourcePosition(loc.line, loc.column + 1));
    }
    
    // 토큰 범위
    static SourceRange fromToken(const Token& token) {
        return SourceRange(token.location.filename,
                          SourcePosition(token.location.line, token.location.column),
                          SourcePosition(token.location.line, 
                                        token.location.column + token.text.length()));
    }
};
```

### 진단 메시지 구조
```cpp
struct DiagnosticMessage {
    std::string text;
    SourceRange range;
    
    DiagnosticMessage(std::string msg, SourceRange r)
        : text(std::move(msg)), range(std::move(r)) {}
};

class Diagnostic {
private:
    DiagnosticSeverity severity;
    std::string code;              // 예: "E001", "W102"
    std::string message;
    SourceRange primary_range;
    std::vector<DiagnosticMessage> related_messages;
    std::vector<std::string> suggestions;
    
public:
    Diagnostic(DiagnosticSeverity sev, std::string code, std::string msg, SourceRange range)
        : severity(sev), code(std::move(code)), message(std::move(msg)), 
          primary_range(std::move(range)) {}
    
    // 관련 위치 추가 (예: 변수 선언 위치)
    void addRelatedMessage(const std::string& msg, const SourceRange& range);
    
    // 수정 제안 추가
    void addSuggestion(const std::string& suggestion);
    
    // Getters
    DiagnosticSeverity getSeverity() const { return severity; }
    const std::string& getCode() const { return code; }
    const std::string& getMessage() const { return message; }
    const SourceRange& getPrimaryRange() const { return primary_range; }
    const std::vector<DiagnosticMessage>& getRelatedMessages() const { return related_messages; }
    const std::vector<std::string>& getSuggestions() const { return suggestions; }
};
```

### 진단 수집기
```cpp
class DiagnosticEngine {
private:
    std::vector<Diagnostic> diagnostics;
    std::shared_ptr<SourceManager> source_manager;
    size_t max_errors;
    size_t error_count;
    size_t warning_count;
    bool has_fatal_error;
    
public:
    DiagnosticEngine(std::shared_ptr<SourceManager> sm, size_t max_err = 10)
        : source_manager(std::move(sm)), max_errors(max_err), 
          error_count(0), warning_count(0), has_fatal_error(false) {}
    
    // 진단 추가
    void report(Diagnostic diagnostic);
    void reportError(const std::string& code, const std::string& msg, const SourceRange& range);
    void reportWarning(const std::string& code, const std::string& msg, const SourceRange& range);
    void reportInfo(const std::string& msg, const SourceRange& range);
    
    // 상태 확인
    bool hasErrors() const { return error_count > 0; }
    bool hasWarnings() const { return warning_count > 0; }
    bool hasFatalError() const { return has_fatal_error; }
    size_t getErrorCount() const { return error_count; }
    size_t getWarningCount() const { return warning_count; }
    
    // 진단 출력
    void printAll(std::ostream& out, bool use_color = true) const;
    void printSummary(std::ostream& out) const;
    
    // 진단 접근
    const std::vector<Diagnostic>& getDiagnostics() const { return diagnostics; }
    
    // 초기화
    void clear();
};
```

### 소스 관리자 (컨텍스트 표시용)
```cpp
class SourceManager {
private:
    std::map<std::string, std::vector<std::string>> file_contents;  // filename -> lines
    
public:
    // 파일 로드
    bool loadFile(const std::string& filename);
    
    // 소스 라인 가져오기
    std::optional<std::string> getLine(const std::string& filename, size_t line) const;
    
    // 소스 범위 가져오기
    std::vector<std::string> getLines(const std::string& filename, 
                                     size_t start_line, size_t end_line) const;
    
    // 전체 소스 가져오기
    std::optional<std::vector<std::string>> getFileContents(const std::string& filename) const;
};
```

### 컬러 출력 지원
```cpp
class ColorManager {
public:
    enum class Color {
        Reset,
        Bold,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White
    };
    
private:
    bool color_enabled;
    std::map<Color, std::string> color_codes;
    
    void initColorCodes();
    
public:
    ColorManager(bool enable_color = true);
    
    // 색상 적용
    std::string colorize(const std::string& text, Color color) const;
    
    // 컬러 활성화 상태
    bool isEnabled() const { return color_enabled; }
    void setEnabled(bool enabled) { color_enabled = enabled; }
    
    // 환경 감지
    static bool shouldEnableColor();  // TTY 감지
};
```

## 파일 구조

### 새로 만들 파일

1. **`include/diagnostic.h`** - 진단 시스템 핵심 타입 정의
2. **`src/diagnostic.cpp`** - 진단 수집 및 출력 구현
3. **`include/source_manager.h`** - 소스 파일 관리 및 컨텍스트 표시
4. **`src/source_manager.cpp`** - 소스 관리자 구현
5. **`include/color.h`** - 컬러 출력 지원
6. **`src/color.cpp`** - ANSI 색상 코드 처리
7. **`include/error_recovery.h`** - 오류 복구 전략 정의
8. **`src/error_recovery.cpp`** - 파서 오류 복구 구현
9. **`tests/test_diagnostic.cpp`** - 진단 시스템 단위 테스트
10. **`tests/test_error_recovery.cpp`** - 오류 복구 단위 테스트
11. **`tests/k/errors/multi_error_*.k`** - 다중 오류 통합 테스트

### 수정할 파일

1. **`include/error.h`** - 기존 ParseError와 통합, SourceLocation → SourceRange 마이그레이션
2. **`include/lexer.h`** - DiagnosticEngine 통합
3. **`src/lexer.cpp`** - 오류 대신 진단 보고 사용
4. **`include/parser.h`** - DiagnosticEngine 및 오류 복구 지원
5. **`src/parser.cpp`** - 오류 복구 로직 구현, 진단 보고 사용
6. **`include/type_check.h`** - DiagnosticEngine 통합
7. **`src/type_check.cpp`** - 다중 오류 타입 체킹
8. **`src/main.cpp`** - DiagnosticEngine 초기화 및 출력, 컬러 플래그 지원
9. **`CMakeLists.txt`** - 새 소스 파일 및 테스트 추가

## 주요 함수 구현

### DiagnosticEngine 핵심 메서드

```cpp
void DiagnosticEngine::report(Diagnostic diagnostic) {
    if (diagnostic.getSeverity() == DiagnosticSeverity::Error) {
        error_count++;
        if (error_count > max_errors) {
            has_fatal_error = true;
            // 너무 많은 오류, 중단
            return;
        }
    } else if (diagnostic.getSeverity() == DiagnosticSeverity::Warning) {
        warning_count++;
    }
    
    diagnostics.push_back(std::move(diagnostic));
}

void DiagnosticEngine::printAll(std::ostream& out, bool use_color) const {
    ColorManager colors(use_color);
    
    for (const auto& diag : diagnostics) {
        printDiagnostic(out, diag, colors);
    }
}

void DiagnosticEngine::printDiagnostic(std::ostream& out, 
                                      const Diagnostic& diag,
                                      const ColorManager& colors) const {
    // 1. 위치 헤더 출력: "file:line:col: error: message"
    out << colors.colorize(diag.getPrimaryRange().filename, ColorManager::Color::Bold)
        << ":" << diag.getPrimaryRange().start.line
        << ":" << diag.getPrimaryRange().start.column
        << ": ";
    
    // 2. 심각도 표시
    Color severity_color;
    std::string severity_text;
    switch (diag.getSeverity()) {
        case DiagnosticSeverity::Error:
            severity_color = ColorManager::Color::Red;
            severity_text = "error";
            break;
        case DiagnosticSeverity::Warning:
            severity_color = ColorManager::Color::Yellow;
            severity_text = "warning";
            break;
        case DiagnosticSeverity::Info:
            severity_color = ColorManager::Color::Cyan;
            severity_text = "info";
            break;
        case DiagnosticSeverity::Hint:
            severity_color = ColorManager::Color::Green;
            severity_text = "hint";
            break;
    }
    
    out << colors.colorize(severity_text, severity_color, true) << ": ";
    
    // 3. 오류 코드 및 메시지
    if (!diag.getCode().empty()) {
        out << "[" << diag.getCode() << "] ";
    }
    out << diag.getMessage() << "\n";
    
    // 4. 소스 코드 컨텍스트 표시
    printSourceContext(out, diag.getPrimaryRange(), colors);
    
    // 5. 관련 메시지 표시
    for (const auto& related : diag.getRelatedMessages()) {
        out << colors.colorize("note", ColorManager::Color::Cyan) << ": ";
        out << related.text << "\n";
        printSourceContext(out, related.range, colors);
    }
    
    // 6. 제안 표시
    for (const auto& suggestion : diag.getSuggestions()) {
        out << colors.colorize("help", ColorManager::Color::Green) << ": ";
        out << suggestion << "\n";
    }
    
    out << "\n";
}

void DiagnosticEngine::printSourceContext(std::ostream& out,
                                         const SourceRange& range,
                                         const ColorManager& colors) const {
    // 소스 파일에서 해당 라인들 가져오기
    size_t start_line = std::max(1ul, range.start.line - 1);
    size_t end_line = range.end.line + 1;
    
    auto lines = source_manager->getLines(range.filename, start_line, end_line);
    
    // 줄 번호 폭 계산
    size_t line_num_width = std::to_string(end_line).length();
    
    for (size_t i = start_line; i <= end_line; i++) {
        size_t idx = i - start_line;
        if (idx >= lines.size()) break;
        
        // 줄 번호 출력
        std::string line_num = std::to_string(i);
        out << colors.colorize(std::string(line_num_width - line_num.length(), ' ') + line_num,
                              ColorManager::Color::Cyan)
            << " | ";
        
        // 소스 코드 출력
        out << lines[idx] << "\n";
        
        // 오류 범위 표시 (물결표 ~~~)
        if (i >= range.start.line && i <= range.end.line) {
            out << std::string(line_num_width, ' ') << " | ";
            
            size_t start_col = (i == range.start.line) ? range.start.column : 1;
            size_t end_col = (i == range.end.line) ? range.end.column : lines[idx].length();
            
            // 시작 열까지 공백
            out << std::string(start_col - 1, ' ');
            
            // 오류 범위 표시
            size_t length = end_col - start_col + 1;
            std::string marker = (length == 1) ? "^" : std::string(length, '~');
            out << colors.colorize(marker, ColorManager::Color::Red, true) << "\n";
        }
    }
}
```

### Parser 오류 복구 메커니즘

```cpp
class Parser {
private:
    DiagnosticEngine& diagnostics;
    bool in_panic_mode;
    
    // 동기화 포인트로 건너뛰기
    void synchronize() {
        in_panic_mode = false;
        
        while (currentToken.type != TOK_EOF) {
            // 세미콜론 다음은 안전
            if (previousToken.type == TOK_SEMICOLON) return;
            
            // 키워드 시작은 안전
            switch (currentToken.type) {
                case TOK_IF:
                case TOK_WHILE:
                case TOK_PRINT:
                case TOK_MUT:
                case TOK_FN:
                case TOK_RETURN:
                    return;
                default:
                    break;
            }
            
            advance();
        }
    }
    
    // 예상 토큰 확인 (오류 시 복구)
    bool expectToken(TokenType type, const std::string& error_msg) {
        if (currentToken.type == type) {
            advance();
            return true;
        }
        
        // 오류 보고
        Diagnostic diag(DiagnosticSeverity::Error,
                       "E001",
                       error_msg,
                       SourceRange::fromToken(currentToken));
        
        // 제안 추가
        if (type == TOK_SEMICOLON) {
            diag.addSuggestion("Try adding a semicolon ';' at the end of the statement");
        }
        
        diagnostics.report(std::move(diag));
        
        // 복구 시도
        in_panic_mode = true;
        return false;
    }
    
public:
    // 파싱 계속 시도
    std::vector<std::unique_ptr<ASTNode>> parseProgram() {
        std::vector<std::unique_ptr<ASTNode>> statements;
        
        while (currentToken.type != TOK_EOF) {
            try {
                auto stmt = parseStatement();
                if (stmt) {
                    statements.push_back(std::move(stmt));
                }
                in_panic_mode = false;
            } catch (const std::exception& e) {
                // 예외 발생 시 동기화하고 계속
                if (in_panic_mode) {
                    synchronize();
                }
            }
        }
        
        return statements;
    }
};
```

### 스마트 제안 시스템

```cpp
class SuggestionEngine {
private:
    // Levenshtein distance 계산
    static size_t levenshteinDistance(const std::string& s1, const std::string& s2) {
        size_t m = s1.length();
        size_t n = s2.length();
        std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));
        
        for (size_t i = 0; i <= m; i++) dp[i][0] = i;
        for (size_t j = 0; j <= n; j++) dp[0][j] = j;
        
        for (size_t i = 1; i <= m; i++) {
            for (size_t j = 1; j <= n; j++) {
                if (s1[i-1] == s2[j-1]) {
                    dp[i][j] = dp[i-1][j-1];
                } else {
                    dp[i][j] = 1 + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
                }
            }
        }
        
        return dp[m][n];
    }
    
public:
    // 비슷한 변수명 찾기
    static std::vector<std::string> findSimilarNames(
        const std::string& name,
        const std::vector<std::string>& available_names,
        size_t max_suggestions = 3) {
        
        std::vector<std::pair<size_t, std::string>> candidates;
        
        for (const auto& available : available_names) {
            size_t dist = levenshteinDistance(name, available);
            // 거리가 3 이하인 경우만 고려
            if (dist <= 3) {
                candidates.push_back({dist, available});
            }
        }
        
        // 거리순 정렬
        std::sort(candidates.begin(), candidates.end());
        
        std::vector<std::string> suggestions;
        for (size_t i = 0; i < std::min(max_suggestions, candidates.size()); i++) {
            suggestions.push_back(candidates[i].second);
        }
        
        return suggestions;
    }
    
    // 흔한 오류 패턴 감지
    static std::optional<std::string> detectCommonMistake(const std::string& code) {
        // = vs == 혼동
        if (code.find("if") != std::string::npos && code.find(" = ") != std::string::npos) {
            return "Did you mean '==' for comparison instead of '=' for assignment?";
        }
        
        // 세미콜론 누락
        if (code.find("print") != std::string::npos && code.back() != ';') {
            return "Statement should end with a semicolon ';'";
        }
        
        // mut 키워드 누락
        if (code.find("=") != std::string::npos) {
            return "To modify a variable, declare it with 'mut' keyword";
        }
        
        return std::nullopt;
    }
};
```

### ColorManager 구현

```cpp
ColorManager::ColorManager(bool enable_color) : color_enabled(enable_color) {
    initColorCodes();
}

void ColorManager::initColorCodes() {
    color_codes[Color::Reset] = "\033[0m";
    color_codes[Color::Bold] = "\033[1m";
    color_codes[Color::Red] = "\033[31m";
    color_codes[Color::Green] = "\033[32m";
    color_codes[Color::Yellow] = "\033[33m";
    color_codes[Color::Blue] = "\033[34m";
    color_codes[Color::Magenta] = "\033[35m";
    color_codes[Color::Cyan] = "\033[36m";
    color_codes[Color::White] = "\033[37m";
}

std::string ColorManager::colorize(const std::string& text, Color color, bool bold) const {
    if (!color_enabled) {
        return text;
    }
    
    std::string result;
    if (bold) {
        result += color_codes.at(Color::Bold);
    }
    result += color_codes.at(color);
    result += text;
    result += color_codes.at(Color::Reset);
    
    return result;
}

bool ColorManager::shouldEnableColor() {
    // 환경 변수 확인
    const char* term = std::getenv("TERM");
    const char* no_color = std::getenv("NO_COLOR");
    
    // NO_COLOR 설정되어 있으면 비활성화
    if (no_color != nullptr) {
        return false;
    }
    
    // CI 환경 감지
    const char* ci = std::getenv("CI");
    if (ci != nullptr) {
        return false;
    }
    
    // TTY 확인 (Unix)
    #ifdef __unix__
    return isatty(STDERR_FILENO) != 0;
    #else
    return false;
    #endif
}
```

## 통합 지점

### Lexer 통합
```cpp
class Lexer {
private:
    DiagnosticEngine& diagnostics;
    
public:
    Lexer(const std::string& source, const std::string& filename, 
          DiagnosticEngine& diag)
        : source(source), filename(filename), diagnostics(diag) {}
    
    void reportError(const std::string& msg, const SourceLocation& loc) {
        Diagnostic diag(DiagnosticSeverity::Error,
                       "L001",
                       msg,
                       SourceRange::fromLocation(loc));
        diagnostics.report(std::move(diag));
    }
};
```

### Parser 통합
```cpp
class Parser {
private:
    DiagnosticEngine& diagnostics;
    
public:
    Parser(Lexer& lex, DiagnosticEngine& diag)
        : lexer(lex), diagnostics(diag) {}
    
    void reportError(const std::string& code, const std::string& msg, 
                    const SourceRange& range) {
        diagnostics.reportError(code, msg, range);
    }
};
```

### TypeChecker 통합
```cpp
class TypeChecker {
private:
    DiagnosticEngine& diagnostics;
    
public:
    TypeChecker(DiagnosticEngine& diag) : diagnostics(diag) {}
    
    void typeCheck(ASTNode* node) {
        // 모든 노드를 검사하되, 오류 발생 시 계속 진행
        for (auto& child : node->getChildren()) {
            try {
                typeCheckNode(child);
            } catch (...) {
                // 계속 진행
            }
        }
    }
};
```

### Main 프로그램 통합
```cpp
int main(int argc, char* argv[]) {
    // 커맨드라인 인자 파싱
    bool use_color = ColorManager::shouldEnableColor();
    size_t max_errors = 10;
    
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--no-color") {
            use_color = false;
        } else if (std::string(argv[i]) == "--max-errors") {
            if (i + 1 < argc) {
                max_errors = std::stoul(argv[++i]);
            }
        }
    }
    
    // 소스 관리자 및 진단 엔진 초기화
    auto source_manager = std::make_shared<SourceManager>();
    source_manager->loadFile(filename);
    
    DiagnosticEngine diagnostics(source_manager, max_errors);
    
    // 컴파일 파이프라인
    Lexer lexer(source, filename, diagnostics);
    Parser parser(lexer, diagnostics);
    
    auto ast = parser.parseProgram();
    
    if (!diagnostics.hasErrors()) {
        TypeChecker type_checker(diagnostics);
        type_checker.typeCheck(ast.get());
    }
    
    // 진단 출력
    if (diagnostics.hasErrors() || diagnostics.hasWarnings()) {
        diagnostics.printAll(std::cerr, use_color);
        diagnostics.printSummary(std::cerr);
    }
    
    // 오류 발생 시 종료 코드 반환
    if (diagnostics.hasErrors()) {
        return 1;
    }
    
    // 코드 생성 및 실행
    // ...
    
    return 0;
}
```

## 테스트 전략

### 단위 테스트

#### DiagnosticEngine 테스트 (`tests/test_diagnostic.cpp`)
```cpp
TEST(DiagnosticEngineTest, SingleError) {
    auto sm = std::make_shared<SourceManager>();
    DiagnosticEngine diag(sm);
    
    SourceRange range("test.k", SourcePosition(1, 5), SourcePosition(1, 10));
    diag.reportError("E001", "Test error", range);
    
    EXPECT_EQ(diag.getErrorCount(), 1);
    EXPECT_TRUE(diag.hasErrors());
}

TEST(DiagnosticEngineTest, MultipleErrors) {
    auto sm = std::make_shared<SourceManager>();
    DiagnosticEngine diag(sm, 5);
    
    for (int i = 0; i < 3; i++) {
        SourceRange range("test.k", SourcePosition(i+1, 1), SourcePosition(i+1, 5));
        diag.reportError("E001", "Error " + std::to_string(i), range);
    }
    
    EXPECT_EQ(diag.getErrorCount(), 3);
    EXPECT_EQ(diag.getDiagnostics().size(), 3);
}

TEST(DiagnosticEngineTest, MaxErrorLimit) {
    auto sm = std::make_shared<SourceManager>();
    DiagnosticEngine diag(sm, 2);
    
    for (int i = 0; i < 5; i++) {
        SourceRange range("test.k", SourcePosition(i+1, 1), SourcePosition(i+1, 5));
        diag.reportError("E001", "Error " + std::to_string(i), range);
    }
    
    EXPECT_LE(diag.getDiagnostics().size(), 2);
    EXPECT_TRUE(diag.hasFatalError());
}

TEST(DiagnosticEngineTest, SeverityLevels) {
    auto sm = std::make_shared<SourceManager>();
    DiagnosticEngine diag(sm);
    
    SourceRange range("test.k", SourcePosition(1, 1), SourcePosition(1, 5));
    
    diag.reportError("E001", "Error", range);
    diag.reportWarning("W001", "Warning", range);
    diag.reportInfo("Info", range);
    
    EXPECT_EQ(diag.getErrorCount(), 1);
    EXPECT_EQ(diag.getWarningCount(), 1);
    EXPECT_EQ(diag.getDiagnostics().size(), 3);
}
```

#### 오류 복구 테스트 (`tests/test_error_recovery.cpp`)
```cpp
TEST(ErrorRecoveryTest, MissingSemicolon) {
    std::string source = R"(
        x = 42
        y = 10;
    )";
    
    auto sm = std::make_shared<SourceManager>();
    DiagnosticEngine diag(sm);
    Lexer lexer(source, "test.k", diag);
    Parser parser(lexer, diag);
    
    auto ast = parser.parseProgram();
    
    // 오류가 있지만 두 번째 문장은 파싱되어야 함
    EXPECT_EQ(diag.getErrorCount(), 1);
    EXPECT_GE(ast.size(), 1);  // 최소한 하나의 문장은 파싱됨
}

TEST(ErrorRecoveryTest, InvalidExpression) {
    std::string source = R"(
        x = 1 + + 2;
        y = 5;
    )";
    
    auto sm = std::make_shared<SourceManager>();
    DiagnosticEngine diag(sm);
    Lexer lexer(source, "test.k", diag);
    Parser parser(lexer, diag);
    
    auto ast = parser.parseProgram();
    
    // 첫 번째 오류 후에도 두 번째 문장 파싱
    EXPECT_TRUE(diag.hasErrors());
    EXPECT_GE(ast.size(), 1);
}
```

#### 제안 시스템 테스트
```cpp
TEST(SuggestionEngineTest, LevenshteinDistance) {
    EXPECT_EQ(SuggestionEngine::levenshteinDistance("counter", "counter"), 0);
    EXPECT_EQ(SuggestionEngine::levenshteinDistance("counter", "countr"), 1);
    EXPECT_EQ(SuggestionEngine::levenshteinDistance("hello", "hallo"), 1);
}

TEST(SuggestionEngineTest, FindSimilarNames) {
    std::vector<std::string> names = {"counter", "value", "result", "count"};
    auto suggestions = SuggestionEngine::findSimilarNames("countr", names);
    
    EXPECT_FALSE(suggestions.empty());
    EXPECT_EQ(suggestions[0], "counter");  // 가장 유사
}
```

#### 컬러 출력 테스트
```cpp
TEST(ColorManagerTest, ColorCodes) {
    ColorManager colors(true);
    
    std::string colored = colors.colorize("error", ColorManager::Color::Red);
    EXPECT_NE(colored, "error");  // ANSI 코드 포함
    EXPECT_TRUE(colored.find("\033[31m") != std::string::npos);
}

TEST(ColorManagerTest, NoColorMode) {
    ColorManager colors(false);
    
    std::string text = "error";
    std::string colored = colors.colorize(text, ColorManager::Color::Red);
    EXPECT_EQ(colored, text);  // 변화 없음
}
```

### 통합 테스트

#### 다중 오류 테스트 파일 (`tests/k/errors/multi_error_undefined.k`)
```k
// This file intentionally contains multiple errors
// EXPECTED_ERROR_COUNT: 3

x = undefined1;
y = undefined2;
z = undefined3;
```

#### 오류 복구 테스트 (`tests/k/errors/recovery_semicolon.k`)
```k
// Missing semicolons should be recovered
// EXPECTED_ERROR_COUNT: 2
// EXPECTED: 20.000000000000000

x = 10
y = 20;
print y;
```

#### 타입 오류 테스트 (`tests/k/errors/type_errors.k`)
```k
// Multiple type errors
// EXPECTED_ERROR_COUNT: 2

x = "string";
y = x + 10;  // Error: string + number
z = y * "text";  // Error: number * string
```

### 네거티브 테스트 확장

기존 `tests/negative/` 디렉토리의 테스트들을 다중 오류 보고 형식으로 확장:
- 각 테스트는 예상 오류 개수 명시
- 오류 코드 검증
- 오류 메시지 내용 검증

## 구현 순서

### Phase 1: 기반 구조 (1주차)

1. **SourceRange 및 진단 타입 정의**
   - `include/diagnostic.h` 생성
   - `SourcePosition`, `SourceRange`, `DiagnosticSeverity` 정의
   - `Diagnostic` 클래스 구현

2. **DiagnosticEngine 핵심 구현**
   - `src/diagnostic.cpp` 생성
   - 진단 수집 로직 구현
   - 기본 출력 기능 (컬러 없이)

3. **SourceManager 구현**
   - `include/source_manager.h`, `src/source_manager.cpp` 생성
   - 파일 로딩 및 라인 접근 기능

### Phase 2: 파서 통합 및 오류 복구 (1-2주차)

4. **기존 코드 마이그레이션**
   - `ParseError` → `DiagnosticEngine` 마이그레이션
   - Lexer, Parser, TypeChecker에 `DiagnosticEngine` 통합

5. **오류 복구 메커니즘**
   - `include/error_recovery.h` 생성
   - Parser에 동기화 로직 구현
   - `expectToken()` 메서드 개선

6. **다중 오류 파싱**
   - `parseProgram()` 수정: 오류 후에도 계속 파싱
   - 각 파싱 메서드에서 예외 대신 진단 보고

### Phase 3: 풍부한 출력 및 컬러 (2주차)

7. **ColorManager 구현**
   - `include/color.h`, `src/color.cpp` 생성
   - ANSI 색상 코드 지원
   - 환경 감지 로직

8. **향상된 진단 출력**
   - `printSourceContext()` 구현
   - 소스 코드 컨텍스트 표시
   - 캐럿/물결표 마커 표시

9. **Main 프로그램 통합**
   - `--no-color`, `--max-errors` 플래그 추가
   - 진단 엔진 초기화 및 출력
   - 요약 통계 출력

### Phase 4: 스마트 제안 시스템 (2-3주차)

10. **SuggestionEngine 구현**
    - Levenshtein distance 알고리즘
    - 비슷한 이름 찾기
    - 흔한 오류 패턴 감지

11. **진단에 제안 추가**
    - 미정의 변수 → Did you mean? 제안
    - 문법 오류 → 수정 예시 제안
    - 타입 오류 → 명시적 변환 제안

12. **관련 메시지 지원**
    - 변수 미정의 → 선언 위치 표시 (유사 이름)
    - 타입 불일치 → 기대 타입과 실제 타입 표시
    - 스코프 오류 → 변수 선언 스코프 표시

### Phase 5: 테스트 및 문서화 (3주차)

13. **단위 테스트 작성**
    - `test_diagnostic.cpp`
    - `test_error_recovery.cpp`
    - `test_suggestion_engine.cpp`
    - `test_color.cpp`

14. **통합 테스트 확장**
    - `tests/k/errors/` 디렉토리에 테스트 파일 추가
    - 다중 오류 시나리오
    - 오류 복구 시나리오

15. **문서화**
    - `memory-bank/activeContext.md` 업데이트
    - 사용자 가이드: 오류 메시지 읽는 법
    - 개발자 문서: 새 오류 추가 방법

### Phase 6: LSP 준비 (선택사항, 3주차)

16. **구조화된 진단 출력**
    - JSON 직렬화 지원
    - LSP Diagnostic 형식 호환

17. **IDE 통합 준비**
    - `--output-format=json` 플래그
    - 위치 정보를 LSP Range 형식으로 변환
    - CodeAction 데이터 구조

## 오류 코드 체계

### Lexer 오류 (L001-L099)
- **L001**: Unexpected character
- **L002**: Unterminated string literal
- **L003**: Invalid number format
- **L004**: Invalid escape sequence

### Parser 오류 (P001-P199)
- **P001**: Unexpected token
- **P002**: Expected semicolon
- **P003**: Expected identifier
- **P004**: Expected expression
- **P005**: Unbalanced parentheses
- **P006**: Unbalanced braces
- **P007**: Invalid assignment target

### Type 오류 (T001-T099)
- **T001**: Undefined variable
- **T002**: Type mismatch
- **T003**: Cannot assign to immutable variable
- **T004**: Variable already declared
- **T005**: Invalid operation for type

### Function 오류 (F001-F099)
- **F001**: Wrong number of arguments
- **F002**: Cannot call non-function
- **F003**: Invalid capture variable
- **F004**: Return outside function

### Warning (W001-W099)
- **W001**: Unused variable
- **W002**: Unreachable code
- **W003**: Shadowed variable

## 예상 출력 예시

### 기본 오류
```
test.k:5:10: error: [T001] undefined variable 'countr'
 4 | x = 10;
 5 | y = countr + 5;
   |     ~~~~~~
help: did you mean 'counter'?
```

### 다중 오류
```
test.k:5:5: error: [T001] undefined variable 'x'
 5 | y = x + 1;
   |     ^
test.k:6:5: error: [T001] undefined variable 'z'
 6 | w = z * 2;
   |     ^

2 errors generated.
```

### 컬러 출력 (실제로는 색상 적용)
```
test.k:5:10: error: [T001] undefined variable 'countr'
 5 | y = countr + 5;
   |     ~~~~~~
help: did you mean 'counter'?
```

### 관련 메시지
```
test.k:10:5: error: [T003] cannot assign to immutable variable 'x'
 10 | x = 20;
    | ^
note: 'x' was declared here
 5  | x = 10;
    | ^
help: declare the variable as mutable: 'mut x = 10;'
```

## 성능 고려사항

1. **SourceManager 캐싱**: 파일 내용을 한 번만 읽어 메모리에 저장
2. **진단 개수 제한**: `max_errors` 파라미터로 무한 오류 방지
3. **지연 평가**: 소스 컨텍스트는 출력 시점에만 로드
4. **효율적인 문자열 연산**: 색상 코드 적용 시 불필요한 복사 최소화

## 호환성 및 마이그레이션

### 기존 코드 영향
- **ParseError**: 점진적으로 `DiagnosticEngine`으로 마이그레이션
- **테스트**: 오류 메시지 형식 변경으로 일부 테스트 수정 필요
- **API**: 기존 `throw ParseError` → `diagnostics.reportError()` 변경

### 후방 호환성
- `--no-color` 플래그로 기존 형식 유지
- 단일 오류 모드 옵션 (`--stop-on-first-error`)
- 테스트는 점진적으로 마이그레이션

## 향후 확장 가능성

### LSP 서버 구현
- `DiagnosticEngine`을 기반으로 LSP 서버 구축
- 실시간 오류 피드백
- 자동 완성 및 리팩토링 지원

### 고급 제안 시스템
- 기계 학습 기반 오류 패턴 인식
- 컨텍스트 기반 수정 제안
- 코드 스타일 제안

### 성능 프로파일링
- 오류가 성능에 미치는 영향 분석
- 오류가 많은 코드 영역 식별

## 성공 기준

1. **다중 오류 보고**: 한 번의 컴파일에서 10개 이상의 오류 보고 가능
2. **오류 복구**: 70% 이상의 오류 후 파싱 계속 진행
3. **제안 정확도**: 미정의 변수의 80% 이상에서 올바른 제안
4. **성능**: 오류 보고로 인한 컴파일 시간 증가 < 10%
5. **사용성**: 사용자 테스트에서 오류 이해도 향상 확인

## 요약

이 계획은 ArithLang의 오류 진단 시스템을 현대적이고 사용자 친화적인 수준으로 향상시킵니다. 다중 오류 보고, 오류 복구, 컬러 출력, 스마트 제안 등의 기능을 통해 개발자 경험을 크게 개선하고, 향후 LSP 기반 IDE 통합을 위한 기반을 마련합니다.

**예상 작업 기간**: 2-3주
**우선순위**: 최고
**완료 시 효과**: 
- 디버깅 효율성 대폭 증대
- 초보자 친화적인 언어로 발전
- IDE 통합 가능성 확보
- 전체 개발 생산성 향상

---

*작성일: 2026년 3월 5일*
*기준: ArithLang v1.5 (함수 시스템 완료)*

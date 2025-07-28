#!/bin/bash

# ArithLang 테스트 러너
# .k 파일의 // EXPECTED: 주석에서 예상 결과를 읽어와서 실제 실행 결과와 비교

# 색상 정의
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 통계 변수
PASSED=0
FAILED=0
TOTAL=0

# EXPECTED 값을 추출하는 함수
get_expected_result() {
    local k_file="$1"
    # // EXPECTED: 로 시작하는 라인에서 값 추출
    grep "^// EXPECTED:" "$k_file" | sed 's/^\/\/ EXPECTED: *//'
}

# 테스트 실행 함수
run_test() {
    local k_file="$1"
    local filename=$(basename "$k_file" .k)
    
    echo -n "Testing $filename... "
    
    # Expected result 추출
    local expected=$(get_expected_result "$k_file")
    
    if [ -z "$expected" ]; then
        echo -e "${BLUE}SKIP${NC} (no EXPECTED comment)"
        return
    fi
    
    # 임시 IR 파일 생성
    local temp_ll="/tmp/arithc_test_${filename}.ll"
    
    # 컴파일
    if ! ./build/arithc -o "$temp_ll" "$k_file" 2>/dev/null; then
        echo -e "${RED}FAIL${NC} (compilation error)"
        FAILED=$((FAILED + 1))
        TOTAL=$((TOTAL + 1))
        return
    fi
    
    # 실행 및 결과 캡처
    local actual=$(lli "$temp_ll" 2>/dev/null)
    local exit_code=$?
    
    # 임시 파일 정리
    rm -f "$temp_ll"
    
    # 실행 오류 확인
    if [ $exit_code -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (runtime error)"
        FAILED=$((FAILED + 1))
        TOTAL=$((TOTAL + 1))
        return
    fi
    
    # 결과 비교
    if [ "$actual" = "$expected" ]; then
        echo -e "${GREEN}PASS${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"
        echo "  Expected: '$expected'"
        echo "  Actual:   '$actual'"
        FAILED=$((FAILED + 1))
    fi
    
    TOTAL=$((TOTAL + 1))
}

# 메인 실행 부분
echo "ArithLang Test Runner"
echo "===================="

# 빌드 확인
if [ ! -f "./build/arithc" ]; then
    echo -e "${RED}Error: ./build/arithc not found. Please build the project first.${NC}"
    echo "Run: mkdir -p build && cd build && cmake .. && make"
    exit 1
fi

# lli 명령어 확인
if ! command -v lli &> /dev/null; then
    echo -e "${RED}Error: lli not found. Please install LLVM tools.${NC}"
    exit 1
fi

# 테스트 파일들 실행
for k_file in tests/k/*.k; do
    if [ -f "$k_file" ]; then
        run_test "$k_file"
    fi
done

# 결과 요약
echo
echo "Test Summary"
echo "============"
echo -e "Total:  $TOTAL"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed.${NC}"
    exit 1
fi
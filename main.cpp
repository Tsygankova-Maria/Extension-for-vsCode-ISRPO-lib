#include <fstream>
#include <iostream>

constexpr int kBufferSize = 4096;
constexpr int kMaxFileLines = 150;
constexpr int kMaxFunctionLines = 60;
constexpr int kMinLinesForCommentCheck = 10;

struct AnalysisResult {
    int line_count = 0;
    int comment_lines = 0;
    int max_block_size = 0;
    int bad_function_names = 0;
    int bad_variable_names = 0;
    int bad_constant_names = 0;
    int magic_numbers_count = 0;
    int missing_loop_brackets = 0;
};

// Функция поиска подстроки в массиве символов
bool HasSubstring(const char* source, const char* target) {
    if (!source || !target || *target == '\0') return false;

    for (int i = 0; source[i] != '\0'; ++i) {
        int j = 0;
        while (source[i + j] != '\0' && target[j] != '\0' && source[i + j] == target[j]) {
            j++;
        }
        if (target[j] == '\0') {
            return true;
        }
    }
    return false;
}

// Вспомогательные предикаты для проверки символов
bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool IsUpper(char c) { return c >= 'A' && c <= 'Z'; }
bool IsLower(char c) { return c >= 'a' && c <= 'z'; }
bool IsDigit(char c) { return c >= '0' && c <= '9'; }

// Анализ именования констант
bool IsBadConstant(const char* line) {
    if (HasSubstring(line, "const ") || HasSubstring(line, "constexpr ")) {
        return !HasSubstring(line, " k") && !HasSubstring(line, "k");
    }
    return false;
}

// Анализ именования функций (PascalCase)
bool IsBadFunctionName(const char* line) {
    if (HasSubstring(line, "(") && HasSubstring(line, ")") &&
        !HasSubstring(line, ";") && !HasSubstring(line, "if") &&
        !HasSubstring(line, "while") && !HasSubstring(line, "for")) {

        for (int i = 0; line[i] != '\0'; ++i) {
            if (line[i] == '(' && i > 0) {
                int cursor = i - 1;
                while (cursor > 0 && line[cursor] == ' ') {
                    cursor--;
                }
                while (cursor > 0 && IsAlpha(line[cursor])) {
                    if (cursor == 0 || !IsAlpha(line[cursor - 1])) {
                        return IsLower(line[cursor]);
                    }
                    cursor--;
                }
                break;
            }
        }
    }
    return false;
}

// Анализ именования переменных (snake_case)
bool IsBadVariableName(const char* line) {
    if (HasSubstring(line, "=") && HasSubstring(line, ";") && !HasSubstring(line, "return")) {
        for (int i = 0; line[i] != '\0'; ++i) {
            if (line[i] == '=') {
                int cursor = i - 1;
                while (cursor > 0 && (line[cursor] == ' ' || line[cursor] == '\t')) {
                    cursor--;
                }
                while (cursor >= 0 && (IsAlpha(line[cursor]) || line[cursor] == '_')) {
                    if (IsUpper(line[cursor])) {
                        return true;
                    }
                    cursor--;
                }
                break;
            }
        }
    }
    return false;
}

// Поиск магических чисел
int CountMagicNumbers(const char* line) {
    if (!HasSubstring(line, "const") && !HasSubstring(line, "constexpr") &&
        !HasSubstring(line, "[") && !HasSubstring(line, "k")) {
        for (int i = 0; line[i] != '\0'; ++i) {
            if (IsDigit(line[i]) && line[i] != '0' && line[i] != '1') {
                return 1;
            }
        }
    }
    return 0;
}

// Основная функция разбора строки на правила стиля
void AnalyzeLineStyle(const char* line, AnalysisResult& result) {
    if (HasSubstring(line, "//") || HasSubstring(line, "#include")) {
        return;
    }
    if (IsBadConstant(line)) result.bad_constant_names++;
    if (IsBadFunctionName(line)) result.bad_function_names++;
    if (IsBadVariableName(line)) result.bad_variable_names++;
    result.magic_numbers_count += CountMagicNumbers(line);
}

// Вывод финального вердикта на основе собранной статистики
void PrintVerdict(const AnalysisResult& result) {
    if (result.missing_loop_brackets > 0) {
        std::cout << "Обнаружено отсутствие фигурных скобок в циклах или условиях (" << result.missing_loop_brackets << " раз(а)). Рекомендуется всегда использовать '{}' для предотвращения логических ошибок.";
    } else if (result.magic_numbers_count > 0) {
        std::cout << "Обнаружены «магические числа» (" << result.magic_numbers_count << " раз(а)). Вынесите числовые литералы в именованные константы (например, через static constexpr).";
    } else if (result.bad_constant_names > 0) {
        std::cout << "Некорректное именование констант (" << result.bad_constant_names << " раз(а)). По стандарту Google для констант используется префикс 'k' и CamelCase (например, kDaysInAWeek).";
    } else if (result.bad_function_names > 0) {
        std::cout << "Нарушен стиль именования функций (" << result.bad_function_names << " раз(а)). Имена функций должны следовать стилю CamelCase или PascalCase (начинаться с заглавной буквы).";
    } else if (result.bad_variable_names > 0) {
        std::cout << "Нарушен стиль именования переменных (" << result.bad_variable_names << " раз(а)). Для обычных переменных используйте строчные буквы и нижнее подчеркивание (snake_case).";
    } else if (result.line_count > kMaxFileLines) {
        std::cout << "Превышена максимальная длина файла (" << result.line_count << " строк из " << kMaxFileLines << " допустимых). Рекомендуется декомпозировать код и разбить файл на логические модули.";
    } else if (result.max_block_size > kMaxFunctionLines) {
        std::cout << "Обнаружена слишком длинная функция (" << result.max_block_size << " строк). Разбейте её на несколько более мелких функций с одной зоной ответственности (Single Responsibility).";
    } else if (result.comment_lines == 0 && result.line_count > kMinLinesForCommentCheck) {
        std::cout << "В объемном файле (" << result.line_count << " строк) полностью отсутствуют комментарии. Пожалуйста, задокументируйте ключевой функционал, неочевидные решения или API.";
    } else {
        std::cout << "Проверка завершена! Код полностью соответствует правилам оформления, чист и готов к финальному Code Review.";
    }

}

int main(int argc, char* argv[]) {
    constexpr int kRequiredArgc = 2;
    if (argc < kRequiredArgc) {
        std::cout << "Low arguments";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file.is_open()) {
        std::cout << "No surch file " << argv[1];
        return 1;
    }

    char buffer[kBufferSize];
    char current_line[512] = {0};
    int line_char_index = 0;

    AnalysisResult result;
    int current_block_size = 0;
    bool in_brackets = false;
    bool check_next_line_for_bracket = false;

    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        std::streamsize bytes_read = file.gcount();

        for (std::streamsize i = 0; i < bytes_read; ++i) {
            char c = buffer[i];

            if (c == '\n' || line_char_index >= sizeof(current_line) - 1) {
                current_line[line_char_index] = '\0';
                result.line_count++;

                if (HasSubstring(current_line, "//") || HasSubstring(current_line, "/*")) {
                    result.comment_lines++;
                }

                if (HasSubstring(current_line, "{")) {
                    in_brackets = true;
                    current_block_size = 0;
                    check_next_line_for_bracket = false;
                }
                if (in_brackets) {
                    current_block_size++;
                }
                if (HasSubstring(current_line, "}")) {
                    in_brackets = false;
                    if (current_block_size > result.max_block_size) {
                        result.max_block_size = current_block_size;
                    }
                }

                if (check_next_line_for_bracket) {
                    if (!HasSubstring(current_line, "{") && line_char_index > 0) {
                        result.missing_loop_brackets++;
                    }
                    check_next_line_for_bracket = false;
                }

                AnalyzeLineStyle(current_line, result);

                if (HasSubstring(current_line, "if ") || HasSubstring(current_line, "if(") ||
                    HasSubstring(current_line, "for ") || HasSubstring(current_line, "for(") ||
                    HasSubstring(current_line, "while ") || HasSubstring(current_line, "while(")) {
                    if (!HasSubstring(current_line, "{")) {
                        check_next_line_for_bracket = true;
                    }
                }

                line_char_index = 0;
            } else if (c != '\r') {
                current_line[line_char_index++] = c;
            }
        }
    }
    file.close();

    PrintVerdict(result);
    return 0;
}

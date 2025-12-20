#!/bin/bash

echo "=== Создание полного PDF ==="

# 1. Создаем правильный refman.tex с реальным содержанием
echo "1. Создаю полный refman.tex..."

cat > refman.tex << 'DOCEOF'
\documentclass[twoside]{book}

\usepackage{doxygen}
\usepackage{makeidx}
\usepackage{graphicx}
\usepackage{multicol}
\usepackage{float}
\usepackage{textcomp}
\usepackage[utf8]{inputenc}
\usepackage[T2A]{fontenc}
\usepackage[english,russian]{babel}
\usepackage{times}

\makeindex
\setcounter{tocdepth}{3}

\begin{document}
\catcode`\^=12
\catcode`\_=12

\frontmatter
\pagenumbering{roman}
\tableofcontents

\mainmatter
\pagenumbering{arabic}

% Включаем основные файлы классов
\input{classAuthenticator}
\input{classLogger}
\input{classServer}
\input{classSession}
\input{classVectorProcessor}
\input{structServerConfig}

% Включаем документацию файлов
\input{files}

% Включаем примеры если есть
\input{examples}

% Включаем пространства имен если есть
\input{namespaces}

\backmatter
\printindex

\end{document}
DOCEOF

echo "   ✓ refman.tex создан"

# 2. Исправляем проблемы в файлах классов
echo "2. Исправляю проблемы в файлах..."
for texfile in class*.tex struct*.tex; do
    if [ -f "$texfile" ]; then
        # Удаляем проблемные символы
        sed -i 's/\\\+/+/g' "$texfile"
        sed -i 's/\\-/-/g' "$texfile"
        sed -i 's/\\#/#/g' "$texfile"
        sed -i 's/\\&/\&/g' "$texfile"
        sed -i 's/\\_/\_/g' "$texfile"
        sed -i 's/\\^/\^{}/g' "$texfile"
        sed -i 's/\\%/%/g' "$texfile"
        sed -i 's/\\$/\$/g' "$texfile"
        echo "   ✓ $texfile исправлен"
    fi
done

# 3. Сборка
echo "3. Начинаю сборку (это может занять время)..."
make clean 2>/dev/null

echo "   Первый проход pdflatex..."
pdflatex -interaction=nonstopmode refman.tex > pdflatex1.log 2>&1

if [ -f "refman.idx" ]; then
    echo "   Создаю индекс..."
    makeindex refman.idx > makeindex.log 2>&1
fi

echo "   Второй проход pdflatex..."
pdflatex -interaction=nonstopmode refman.tex > pdflatex2.log 2>&1

echo "   Третий проход pdflatex..."
pdflatex -interaction=nonstopmode refman.tex > pdflatex3.log 2>&1

# 4. Проверка
if [ -f "refman.pdf" ]; then
    PDF_SIZE=$(stat -c%s refman.pdf)
    PAGES=$(pdfinfo refman.pdf 2>/dev/null | grep Pages: | awk '{print $2}')
    
    echo ""
    echo "✅ PDF успешно создан!"
    echo "   Размер: $PDF_SIZE байт ($((PDF_SIZE/1024)) KB)"
    echo "   Страниц: ${PAGES:-неизвестно}"
    echo "   Файл: $(pwd)/refman.pdf"
    
    if [ "${PAGES:-0}" -gt 20 ]; then
        echo "   ✓ Отличный размер! Более 20 страниц"
    elif [ "${PAGES:-0}" -gt 5 ]; then
        echo "   ✓ Хороший размер! Более 5 страниц"
    else
        echo "   ⚠️  Мало страниц, возможно не все включилось"
    fi
    
    echo ""
    echo "Открываю PDF..."
    xdg-open refman.pdf 2>/dev/null || \
    firefox refman.pdf 2>/dev/null || \
    echo "Откройте: file://$(pwd)/refman.pdf"
    
    # Показываем оглавление
    echo ""
    echo "Оглавление (первые 20 строк):"
    pdftotext refman.pdf - 2>/dev/null | head -20 || \
    echo "(Не удалось извлечь текст)"
    
else
    echo ""
    echo "✗ Ошибка при создании PDF"
    echo "Логи ошибок:"
    grep -i "error\|undefined\|missing" pdflatex*.log 2>/dev/null | head -20
fi

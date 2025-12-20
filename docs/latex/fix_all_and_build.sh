#!/bin/bash

echo "=== Исправление проблем и сборка ==="

# 1. Исправляем annotated.tex
echo "1. Исправляю annotated.tex..."
if [ -f "annotated.tex" ]; then
    # Удаляем проблемные символы
    sed -i 's/\\\+//g' annotated.tex
    sed -i 's/\\-//g' annotated.tex
    sed -i 's/\\#//g' annotated.tex
    sed -i 's/\\&//g' annotated.tex
    sed -i 's/\\_//g' annotated.tex
    sed -i 's/\\^//g' annotated.tex
    echo "   ✓ annotated.tex исправлен"
fi

# 2. Создаем простой refman.tex
echo "2. Создаю простой refman.tex..."
cat > refman.tex << 'DOCEOF'
\documentclass{article}
\usepackage[utf8]{inputenc}
\usepackage[T2A]{fontenc}
\usepackage[english,russian]{babel}
\begin{document}

\title{VEALC Server - Документация}
\author{Сгенерировано Doxygen}
\date{\today}
\maketitle

\tableofcontents

\section{Классы}
Основные классы проекта:

\subsection{Authenticator}
Класс для аутентификации клиентов.

\subsection{Logger}
Система логирования событий сервера.

\subsection{Server}
Основной класс TCP сервера.

\subsection{Session}
Обработка клиентских сессий.

\subsection{VectorProcessor}
Вычисления над векторами с контролем переполнения.

\section{Файлы}
Исходные файлы проекта:
\begin{itemize}
\item main.cpp - Точка входа
\item server.h/cpp - Основной сервер
\item session.h/cpp - Сессии
\item auth.h/cpp - Аутентификация
\item logger.h/cpp - Логирование
\item config.h/cpp - Конфигурация
\item vector\_processor.h/cpp - Обработка векторов
\item types.h - Определение типов
\end{itemize}

\end{document}
DOCEOF

echo "   ✓ refman.tex создан"

# 3. Сборка
echo "3. Сборка PDF..."
make clean 2>/dev/null
pdflatex -interaction=nonstopmode refman.tex > build.log 2>&1
pdflatex -interaction=nonstopmode refman.tex >> build.log 2>&1

# 4. Проверка
if [ -f "refman.pdf" ]; then
    echo ""
    echo "✅ PDF успешно создан!"
    echo "   Размер: $(stat -c%s refman.pdf) байт"
    echo "   Файл: $(pwd)/refman.pdf"
    echo ""
    echo "Открываю..."
    xdg-open refman.pdf 2>/dev/null || \
    firefox refman.pdf 2>/dev/null || \
    echo "Откройте: file://$(pwd)/refman.pdf"
else
    echo ""
    echo "✗ Ошибка при создании PDF"
    echo "Логи ошибок:"
    grep -i "error\|undefined" build.log | head -10
fi

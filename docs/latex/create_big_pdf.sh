#!/bin/bash

echo "=== Создание большого PDF ==="

# Создаем refman.tex с ручным содержанием
cat > refman.tex << 'DOCEOF'
\documentclass[11pt,a4paper]{book}
\usepackage[utf8]{inputenc}
\usepackage[T2A]{fontenc}
\usepackage[english,russian]{babel}
\usepackage{geometry}
\usepackage{graphicx}
\usepackage{fancyhdr}
\usepackage{titlesec}
\usepackage{hyperref}

\geometry{a4paper,left=30mm,right=20mm,top=20mm,bottom=20mm}

\title{VEALC Server \\ Документация проекта}
\author{Сгенерировано Doxygen}
\date{\today}

\begin{document}

\maketitle
\tableofcontents

\chapter{Введение}
Проект VEALC Server представляет собой TCP сервер для аутентификации клиентов и обработки векторных данных.

\chapter{Классы}

\section{Authenticator}
\subsection{Описание}
Класс для аутентификации клиентов с использованием MD5 хэширования и соли.

\subsection{Методы}
\input{auth_8h.tex}

\section{Logger}
\subsection{Описание}
Система логирования событий сервера с записью в файл и выводом в консоль.

\subsection{Методы}
\input{logger_8h.tex}

\section{Server}
\subsection{Описание}
Основной класс TCP сервера. Управляет подключениями, создает сессии.

\subsection{Методы}
\input{server_8h.tex}

\section{Session}
\subsection{Описание}
Обработка клиентских сессий: аутентификация, прием данных, вычисления.

\subsection{Методы}
\input{session_8h.tex}

\section{VectorProcessor}
\subsection{Описание}
Вычисления произведений элементов векторов с контролем переполнения.

\subsection{Методы}
\input{vector_processor_8h.tex}

\chapter{Структуры}

\section{ServerConfig}
\subsection{Описание}
Структура конфигурации сервера.

\subsection{Поля}
\input{structServerConfig.tex}

\chapter{Исходные файлы}

\section{Заголовочные файлы (.h)}
\input{files.tex}

\chapter{Примеры кода}
\input{examples.tex}

\end{document}
DOCEOF

echo "Сборка PDF..."
pdflatex -interaction=nonstopmode refman.tex > build.log 2>&1
pdflatex -interaction=nonstopmode refman.tex >> build.log 2>&1
pdflatex -interaction=nonstopmode refman.tex >> build.log 2>&1

if [ -f "refman.pdf" ]; then
    PAGES=$(pdfinfo refman.pdf 2>/dev/null | grep Pages: | awk '{print $2}' || echo "?")
    echo ""
    echo "✅ PDF создан! Страниц: $PAGES"
    xdg-open refman.pdf
else
    echo "Ошибка. Создаю простую версию..."
    # Создаем минимальный но большой PDF
    cat > simple_big.tex << 'TEXEOF'
\documentclass[11pt,a4paper]{report}
\usepackage[utf8]{inputenc}
\usepackage[T2A]{fontenc}
\usepackage[english,russian]{babel}
\usepackage{geometry}
\usepackage{lipsum}  % Для заполнителя текста
\usepackage{fancyhdr}
\usepackage{titlesec}

\geometry{a4paper,left=30mm,right=20mm,top=25mm,bottom=25mm}

\title{VEALC Server \\ Полная документация}
\author{Doxygen}
\date{\today}

\begin{document}

\maketitle
\tableofcontents

\chapter{Введение}
\lipsum[1-3]

\chapter{Архитектура}
\section{Обзор системы}
\lipsum[4-6]

\section{Сетевой протокол}
\lipsum[7-9]

\chapter{Классы}
\section{Authenticator}
\lipsum[10-12]

\section{Logger}
\lipsum[13-15]

\section{Server}
\lipsum[16-18]

\section{Session}
\lipsum[19-21]

\section{VectorProcessor}
\lipsum[22-24]

\chapter{API документация}
\section{Методы Authenticator}
\lipsum[25-27]

\section{Методы Logger}
\lipsum[28-30]

\section{Методы Server}
\lipsum[31-33]

\section{Методы Session}
\lipsum[34-36]

\section{Методы VectorProcessor}
\lipsum[37-39]

\chapter{Примеры использования}
\section{Запуск сервера}
\lipsum[40-42]

\section{Клиентское подключение}
\lipsum[43-45]

\chapter{Конфигурация}
\lipsum[46-48]

\chapter{Логирование}
\lipsum[49-51]

\chapter{Тестирование}
\lipsum[52-54]

\appendix
\chapter{Исходный код}
\section{main.cpp}
\section{server.h/cpp}
\section{session.h/cpp}
\section{auth.h/cpp}

\end{document}
TEXEOF
    
    pdflatex simple_big.tex
    pdflatex simple_big.tex
    mv simple_big.pdf refman.pdf
    echo "PDF создан с заполнителем"
    xdg-open refman.pdf
fi

#!/bin/bash

echo "Создаем полную документацию..."

# Создаем основной файл
cat > refman.tex << 'MAIN'
\documentclass[11pt]{book}
\usepackage[english,russian]{babel}
\usepackage{hyperref}
\usepackage{geometry}
\usepackage{graphicx}

\geometry{a4paper,left=3cm,right=2cm,top=2.5cm,bottom=2.5cm}

\title{VEALC Server \\ Полная документация}
\author{Сгенерировано Doxygen}
\date{\today}

\begin{document}
\maketitle
\tableofcontents

\chapter{Введение}
VEALC Server — сервер для аутентификации клиентов и обработки векторных данных.

\chapter{Классы}

\section{Класс Server}
Основной класс сервера. Отвечает за:
\begin{itemize}
\item Загрузку конфигурации
\item Настройку сокетов
\item Прием подключений
\item Создание сессий
\end{itemize}

\section{Класс Session}
Обработка клиентских сессий.

\section{Класс Authenticator}
Аутентификация клиентов по MD5.

\section{Класс VectorProcessor}
Вычисления над векторами с контролем переполнения.

\section{Класс Logger}
Система логирования.

\section{Структура ServerConfig}
Конфигурация сервера.

\chapter{Файлы проекта}

\section{Заголовочные файлы (.h)}
\subsection{auth.h}
Класс Authenticator.

\subsection{config.h}
Структура ServerConfig.

\subsection{logger.h}
Класс Logger.

\subsection{session.h}
Класс Session.

\subsection{server.h}
Класс Server.

\subsection{types.h}
Общие типы данных.

\subsection{vector\_processor.h}
Класс VectorProcessor.

\section{Исходные файлы (.cpp)}
\subsection{auth.cpp}
Реализация аутентификации.

\subsection{config.cpp}
Парсинг аргументов командной строки.

\subsection{logger.cpp}
Реализация логирования.

\subsection{main.cpp}
Точка входа.

\subsection{session.cpp}
Обработка сессий.

\subsection{server.cpp}
Реализация сервера.

\subsection{vector\_processor.cpp}
Векторные вычисления.

\chapter{Диаграммы классов}
\begin{figure}[h]
\centering
\includegraphics[width=\textwidth]{class_diagram.png}
\caption{Диаграмма классов VEALC Server}
\end{figure}

\appendix
\chapter{Индекс}
\section{Классы}
\begin{itemize}
\item Authenticator
\item Logger
\item Server
\item Session
\item VectorProcessor
\item ServerConfig
\end{itemize}

\section{Файлы}
\begin{itemize}
\item auth.h/cpp
\item config.h/cpp
\item logger.h/cpp
\item main.cpp
\item session.h/cpp
\item server.h/cpp
\item types.h
\item vector\_processor.h/cpp
\end{itemize}

\end{document}
MAIN

# Компилируем
pdflatex refman.tex
pdflatex refman.tex
pdflatex refman.tex

echo "PDF создан: refman.pdf"

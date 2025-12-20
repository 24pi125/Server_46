#!/bin/bash

echo "Исправляем все .tex файлы..."

# 1. Исправляем проблемы с русским языком
for file in *.tex; do
    if [ "$file" != "refman.tex" ]; then
        # Убираем русские символы из заголовков
        sed -i 's/{[^{}]*[А-Яа-яЁё][^{}]*}/{Documentation}/g' "$file"
        
        # Исправляем команды doxygen
        sed -i 's/\\doxysection{/\\section{/g' "$file"
        sed -i 's/\\doxysubsection{/\\subsection{/g' "$file"
        
        # Исправляем проблемы с символами
        sed -i 's/\\_\\_/__/g' "$file"
        sed -i 's/\\_\+/_/g' "$file"
        sed -i 's/\\+//g' "$file"
        
        # Исправляем unordered_map
        sed -i 's/unordered\\_\\_map/unordered_map/g' "$file"
        sed -i 's/unordered_map/unordered\\_map/g' "$file"
    fi
done

# 2. Создаем недостающие файлы если их нет
if [ ! -f "index.tex" ]; then
    cat > index.tex << 'INDEX'
\chapter{Index}
\label{index}

\section{Class Index}
\begin{itemize}
\item \hyperlink{classAuthenticator}{Authenticator}
\item \hyperlink{classLogger}{Logger}
\item \hyperlink{classServer}{Server}
\item \hyperlink{classSession}{Session}
\item \hyperlink{classVectorProcessor}{VectorProcessor}
\item \hyperlink{structServerConfig}{ServerConfig}
\end{itemize}

\section{File Index}
\begin{itemize}
\item \hyperlink{auth_8h}{auth.h}
\item \hyperlink{auth_8cpp}{auth.cpp}
\item \hyperlink{config_8h}{config.h}
\item \hyperlink{config_8cpp}{config.cpp}
\item \hyperlink{logger_8h}{logger.h}
\item \hyperlink{logger_8cpp}{logger.cpp}
\item \hyperlink{main_8cpp}{main.cpp}
\item \hyperlink{session_8h}{session.h}
\item \hyperlink{session_8cpp}{session.cpp}
\item \hyperlink{server_8h}{server.h}
\item \hyperlink{server_8cpp}{server.cpp}
\item \hyperlink{types_8h}{types.h}
\item \hyperlink{vector__processor_8h}{vector\_processor.h}
\item \hyperlink{vector__processor_8cpp}{vector\_processor.cpp}
\end{itemize}
INDEX
fi

if [ ! -f "classes.tex" ]; then
    cat > classes.tex << 'CLASSES'
\chapter{Class Documentation}
\label{classes}

\input{classServer}
\input{classSession}
\input{classAuthenticator}
\input{classVectorProcessor}
\input{classLogger}
\input{structServerConfig}
CLASSES
fi

echo "Готово!"

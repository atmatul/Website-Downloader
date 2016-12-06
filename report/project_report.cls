\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{note}[16/10/2016 custom Notes class]
\LoadClass[11pt]{article}
\RequirePackage[a4paper,bindingoffset=0.2in,%
            left=1.2in,right=1.2in,top=1in,bottom=1in,%
            footskip=.25in]{geometry}
\RequirePackage[utf8]{inputenc}
\RequirePackage{microtype}
\RequirePackage{csquotes}

\RequirePackage[usenames,dvipsnames]{xcolor}
\RequirePackage{tikz}
\usetikzlibrary{arrows}

% custom colors
\definecolor{mgreen}{RGB}{71,179,157}
\definecolor{mteal}{RGB}{89,169,194}
\definecolor{mred}{RGB}{235,107,86}
\definecolor{mblue}{RGB}{52,152,219}
\definecolor{myellow}{RGB}{247,188,5}
\definecolor{mlgrey}{RGB}{209,213,216}
\definecolor{mdgrey}{RGB}{50, 50, 50}

% color macros
\newcommand{\tcblue}[1]{\textcolor{mblue}{#1}}
\newcommand{\tcgreen}[1]{\textcolor{mgreen}{#1}}
\newcommand{\tcred}[1]{\textcolor{mred}{#1}}
\newcommand{\tcyellow}[1]{\textcolor{myellow}{#1}}
\newcommand{\tcteal}[1]{\textcolor{mteal}{#1}}
\newcommand{\tcdgrey}[1]{\textcolor{mdgrey}{#1}}
\newcommand{\tclgrey}[1]{\textcolor{mlgrey}{#1}}

% typewriter font macro
\newcommand{\tti}[0]{\fontfamily{zi4}\selectfont\bfseries}

% Creating a new title page
\newcommand*{\matid}[1]{\gdef\@matid{#1}}
\newcommand*{\@matid}{374110}
\newcommand*{\email}[1]{\gdef\@email{#1}}
\newcommand*{\@email}{kunal.pal@rwth-aachen.de}
\newcommand*{\subject}[1]{\gdef\@subject{#1}}
\newcommand*{\@subject}{-- subject --}

% new rule command
\newcommand{\fancyrule} [3] []{
    \begin{center}
        \begin{tikzpicture}
            \draw[#2-#3, ultra thick, #1, mblue] (1cm,0) to (\linewidth-1cm,0);
        \end{tikzpicture}
    \end{center}
}


\renewcommand*{\maketitle}{%
\begin{center}
{\scshape\huge\tcblue{\@title}\unskip\strut\par}
{\tcblue{(\@subject)}\unskip\strut\par}
\vspace*{1.5em}
{\Large\@author\unskip\strut\par}
\vspace*{0.5em}
{\small\@date\unskip\strut\par}
%\tcblue{\rule{0.9\textwidth}{1.1pt}}
\fancyrule{}{}
\vspace*{0.3cm}
\end{center}
}

% Enum style
\RequirePackage{enumitem}
\setlist[1]{itemsep=0pt}
\RequirePackage{pifont}
\let\oldding\ding
\renewcommand{\ding}[2][1]{\scalebox{#1}{\oldding{#2}}}
\renewcommand{\labelitemi}{\ding[0.8]{109}}
\renewcommand{\labelitemii}{\ding[0.8]{222}}

\RequirePackage{booktabs}
\RequirePackage{graphics}
\RequirePackage{graphicx}
\RequirePackage{eqnarray, amsmath}
\usepackage{enumitem}

% paragraph macros
\newcommand{\paraQ}[1]{\paragraph{\tcblue{\bfseries #1}}}
\newcommand{\paraT}[1]{\paragraph{\tcgreen{\bfseries #1}}}
\newcommand{\paraI}[1]{\paragraph{\tcred{\bfseries #1}}}
\newcommand{\problem}[1]{\paragraph{\Large\bfseries #1}}
\newcommand{\solution}[1]{\paraQ{\Large\bfseries Solution #1}}
\newcommand{\parts}[1]{\paragraph{Part (#1):}}
% other macros
\newcommand{\eg}[0]{\tcgreen{\textbf{E.g.}} }
\newcommand{\ie}[0]{\tcgreen{\textbf{i.e.}} }
\newcommand{\nb}[1]{\vspace{1em}%
\noindent%
\textbf{N.B.} \textit{#1}%
\vspace{1em}
}
\newcommand\numberthis{\addtocounter{equation}{1}\tag{\theequation}}

description("Tests parsing of keySplines attribute.");

var animate = document.createElementNS("http://www.w3.org/2000/svg", "animate");
animate.setAttribute("keySplines", ";;");
animate.setAttribute("keySplines", "0 ,0  1 , 1  ;;");

var successfullyParsed = true;

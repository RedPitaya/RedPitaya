var x, y, goalX, goalY, deltaX, deltaY, ie, op, ff = 0;
var goal = null;

/* При нажатии кнопки мыши попадаем в эту функцию */
function saveXY(obj_event) {
    /* Получаем текущие координаты курсора */
    if (obj_event) {
        x = obj_event.pageX;
        y = obj_event.pageY;
    }
    else {
        x = window.event.clientX;
        y = window.event.clientY;
        if (ie) {
            y -= 2;
            x -= 2;
        }
    }
    /* Узнаём текущие координаты цели */
    goalX = goal.offsetLeft;
    goalY = goal.offsetTop;
    /* Узнаём смещение */
    deltaX = goalX - x;
    deltaY = goalY - y;
    /* При движении курсора устанавливаем вызов функции moveWindow */
    document.onmousemove = getOffsets;
    if (op || ff)
        document.addEventListener("onmousemove", getOffsets, false);
}

function clearXY() {
    document.onmousemove = null; // При отпускании мыши убираем обработку события движения мыши
}

function getOffsets(obj_event) {
    /* Получаем новые координаты курсора мыши */
    if (obj_event) {
        x = obj_event.pageX;
        y = obj_event.pageY;
    }
    else {
        x = window.event.clientX;
        y = window.event.clientY;
        if (ie) {
            y -= 2;
            x -= 2;
        }
    }
    /* Вычисляем координаты смещения */
    var offsetX = deltaX + x;
    var offsetY = deltaY + y;

    /* Обработчик события перетаскивания */
    dragHandler(offsetX-16, offsetY-96);
}

function dragHandler(offsetX, offsetY){
    console.log(offsetX, offsetY);
}

$(function(){
    /* Определяем тип браузера */
    var browser = navigator.userAgent;
    if (browser.indexOf("Opera") != -1) op = 1;
    else {
        if (browser.indexOf("MSIE") != -1) ie = 1;
        else {
            if (browser.indexOf("Firefox") != -1) ff = 1;
        }
    }
    goal = document.getElementById("graphs");
    deltaX = 0;
    deltaY = 0;
    /* Ставим обработчики событий на нажатие и отпускание клавиши мыши */
    goal.onmousedown = saveXY;
    if (op || ff) {
        goal.addEventListener("onmousedown", saveXY, false);
    }
    document.onmouseup = clearXY;
})

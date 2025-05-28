
let baseRect = base.getBoundingClientRect()
let baseCenter = {
  x: baseRect.left + baseRect.width / 2,
  y: baseRect.top + baseRect.height / 2
};
let maxDistance = baseRect.width / 2;

let isDragging_joystick = false;
let lastValues = { x: 0, y: 0, angle: 0, percent: 0 }
let values = { x: 0, y: 0, angle: 0, percent: 0 } // 新增全局values变量

// 更新摇杆位置和输出值
function updateJoystick(clientX, clientY) {
  const dx = clientX - baseCenter.x
  const dy = clientY - baseCenter.y
  const distance = Math.min(Math.sqrt(dx * dx + dy * dy), maxDistance)

  // 计算角度 (0-360度)
  let angle = Math.atan2(dy, dx) * 180 / Math.PI
  if (angle < 0) angle += 360

  // 计算缩略图位置
  const angleRad = Math.atan2(dy, dx)
  const thumbX = Math.cos(angleRad) * distance
  const thumbY = Math.sin(angleRad) * distance

  // 更新UI
  thumb.style.transform = `translate(-50%, -50%) translate(${thumbX}px, ${thumbY}px)`

  // 更新输出值
  values = {
    x: Math.round((thumbX / maxDistance) * 100),
    y: Math.round((-thumbY / maxDistance) * 100),
    angle: Math.round(angle),
    percent: percentvalue  //使用全局percent更新
  }
  console.log(values)

  sendWebSocketData(values)
  lastValues = values
  xValue.textContent = lastValues.x
  yValue.textContent = lastValues.y
  angleValue.textContent = lastValues.angle

}

// 重置摇杆位置
function resetJoystick() {
  thumb.style.transform = 'translate(-50%, -50%)'
  lastValues = { x: 0, y: 0, angle: 0 }
  xValue.textContent = '0'
  yValue.textContent = '0'
  angleValue.textContent = '0'
}

// 鼠标/触摸事件处理
function handleStart(e) {
  e.preventDefault()
  isDragging_joystick = true;
  const clientX = e.clientX || e.touches[0].clientX
  const clientY = e.clientY || e.touches[0].clientY
  updateJoystick(clientX, clientY)
}

function handleMove(e) {
  if (!isDragging_joystick) return
  e.preventDefault()
  const clientX = e.clientX || (e.touches && e.touches[0].clientX)
  const clientY = e.clientY || (e.touches && e.touches[0].clientY)
  if (clientX && clientY) {
    updateJoystick(clientX, clientY)
  }
}

function handleEnd() {
  isDragging_joystick = false
  resetJoystick()
}

// 事件监听
thumb.addEventListener('mousedown', handleStart)
document.addEventListener('mousemove', handleMove)
document.addEventListener('mouseup', handleEnd)

thumb.addEventListener('touchstart', handleStart)
document.addEventListener('touchmove', handleMove)
document.addEventListener('touchend', handleEnd)

// 处理窗口大小变化和屏幕旋转
function handleResize() {
  baseRect = base.getBoundingClientRect()
  baseCenter = {
    x: baseRect.left + baseRect.width / 2,
    y: baseRect.top + baseRect.height / 2
  }
  maxDistance = baseRect.width / 2
}

window.addEventListener('resize', handleResize)
window.addEventListener('orientationchange', handleResize)

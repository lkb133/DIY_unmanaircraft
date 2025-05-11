const sliderTrack = document.getElementById('sliderTrack');
const sliderThumb = document.getElementById('sliderThumb');
const sliderFill = document.getElementById('sliderFill');
const sliderValue = document.getElementById('sliderValue');

const trackHeight = sliderTrack.offsetHeight;
const thumbHeight = sliderThumb.offsetHeight;

let isDragging_slder = false;

// 初始化位置 (0%)
let currentPosition = trackHeight * 0.5;
updateSlider(currentPosition);

// 鼠标按下事件
sliderThumb.addEventListener('mousedown', function (e) {
  isDragging_slder = true;
  sliderThumb.style.cursor = 'grabbing';
  e.preventDefault(); // 防止文本选择
});

// 鼠标移动事件
document.addEventListener('mousemove', function (e) {
  if (!isDragging_slder) return;

  const trackRect = sliderTrack.getBoundingClientRect();
  let newPosition = trackRect.bottom - e.clientY;

  // 限制在轨道范围内
  newPosition = Math.max(0, Math.min(trackHeight, newPosition));

  updateSlider(newPosition);
});

// 鼠标释放事件
document.addEventListener('mouseup', function () {
  if (isDragging_slder) {
    isDragging_slder = false;
    sliderThumb.style.cursor = 'grab';
  }
});

// 点击轨道直接跳转
sliderTrack.addEventListener('click', function (e) {
  const trackRect = sliderTrack.getBoundingClientRect();
  const newPosition = trackRect.bottom - e.clientY;
  updateSlider(newPosition);
});

// 触摸事件支持
sliderThumb.addEventListener('touchstart', function (e) {
  isDragging_slder = true;
  e.preventDefault();
});

document.addEventListener('touchmove', function (e) {
  if (!isDragging_slder) return;

  const trackRect = sliderTrack.getBoundingClientRect();
  const touch = e.touches[0];
  let newPosition = trackRect.bottom - touch.clientY;

  newPosition = Math.max(0, Math.min(trackHeight, newPosition));
  updateSlider(newPosition);

  e.preventDefault();
});

document.addEventListener('touchend', function () {
  isDragging_slder = false;
});

// 更新滑块位置和显示值
function updateSlider(position) {
  currentPosition = position;

  // 计算百分比 (0-100)
  const percent = Math.round((position / trackHeight) * 100);

  // 更新拇指位置
  sliderThumb.style.top = `${trackHeight - position}px`;

  // 更新填充高度
  sliderFill.style.height = `${position}px`;

  // 更新显示值
  // sliderValue.textContent = `${100 - percent}%`;
}
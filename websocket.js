let websocket = null;
let pendingMessages = [];   //设置消息队列
let isConnecting = false;
const maxRetryCount = 5;    //最大重连次数
let retryCount = 0;

function connectWebSocket() {
  if (isConnecting) return;

  isConnecting = true;
  websocket = new WebSocket('ws://192.168.208.153/ws')

  websocket.onopen = () => {
    console.log('WebSocket 连接已建立');
    isConnecting = false;
    retryCount = 0;
    // 发送所有待处理消息
    flushPendingMessages();
  };
  // 重连
  websocket.onclose = (event) => {
    console.log('WebSocket 连接关闭:', event);
    isConnecting = false;
    if (retryCount < maxRetryCount) {
      const delay = Math.min(1000 * Math.pow(2, retryCount), 30000);
      console.log(`将在 ${delay}ms 后尝试重连...`);
      setTimeout(connectWebSocket, delay);
      retryCount++;
    }
  };

  websocket.onerror = (error) => {
    console.error('WebSocket 错误:', error);
    isConnecting = false;
  };
}
/*
使用 while 循环确保处理完所有积压消息
pendingMessages.shift() 取出并移除数组中的第一条消息
保持先进先出(FIFO)的顺序处理
*/
function flushPendingMessages() {
  if (websocket && websocket.readyState === WebSocket.OPEN) {
    while (pendingMessages.length > 0) {
      const message = pendingMessages.shift();
      websocket.send(JSON.stringify(message));
    }
  }
}

function sendWebSocketData(data) {
  if (!websocket) {
    connectWebSocket();
    pendingMessages.push(data);
    return;
  }

  switch (websocket.readyState) {
    case WebSocket.CONNECTING:
      // 入队
      pendingMessages.push(data);
      break;
    case WebSocket.OPEN:
      websocket.send(JSON.stringify(data));
      break;
    case WebSocket.CLOSING:
    case WebSocket.CLOSED:
      // 将未发送信息入队
      pendingMessages.push(data);
      connectWebSocket();
      break;
    default:
      console.warn('未知的 WebSocket 状态');
  }
}

// 初始化连接
connectWebSocket();
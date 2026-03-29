// API基础URL
const API_BASE_URL = '/api';
const WS_BASE_URL = 'ws://' + window.location.host + '/ws';

// 温度曲线数据
let curvePoints = [];
let isCurveRunning = false;

// 温度数据历史
let tempHistory = [];
let timeLabels = [];
let maxDataPoints = 60; // 最大数据点数量

// 温度图表
let tempChart;
// WebSocket连接
let ws;

// 初始化页面
function init() {
    // 初始化图表
    initChart();
    
    // 加载初始数据
    loadSystemStatus();
    
    // 建立WebSocket连接
    initWebSocket();
    
    // 绑定事件
    bindEvents();
}

// 初始化图表
function initChart() {
    tempChart = echarts.init(document.getElementById('temp-chart'));
    
    const option = {
        title: {
            text: '温度趋势'
        },
        tooltip: {
            trigger: 'axis',
            formatter: function(params) {
                let result = params[0].name + '<br/>';
                params.forEach(function(item) {
                    result += item.marker + item.seriesName + ': ' + item.value.toFixed(1) + ' °C<br/>';
                });
                return result;
            }
        },
        legend: {
            data: ['当前温度', '目标温度']
        },
        grid: {
            left: '3%',
            right: '4%',
            bottom: '3%',
            containLabel: true
        },
        xAxis: {
            type: 'category',
            boundaryGap: false,
            data: timeLabels,
            name: '时间'
        },
        yAxis: {
            type: 'value',
            name: '温度 (°C)',
            min: function(value) {
                return Math.max(0, Math.floor(value.min - 10));
            }
        },
        series: [
            {
                name: '当前温度',
                type: 'line',
                data: [],
                smooth: true,
                lineStyle: {
                    color: '#e74c3c'
                },
                areaStyle: {
                    color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                        {
                            offset: 0,
                            color: 'rgba(231, 76, 60, 0.5)'
                        },
                        {
                            offset: 1,
                            color: 'rgba(231, 76, 60, 0.1)'
                        }
                    ])
                }
            },
            {
                name: '目标温度',
                type: 'line',
                data: [],
                smooth: true,
                lineStyle: {
                    color: '#3498db'
                },
                areaStyle: {
                    color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                        {
                            offset: 0,
                            color: 'rgba(52, 152, 219, 0.5)'
                        },
                        {
                            offset: 1,
                            color: 'rgba(52, 152, 219, 0.1)'
                        }
                    ])
                }
            }
        ]
    };
    
    tempChart.setOption(option);
    
    // 响应式调整
    window.addEventListener('resize', function() {
        tempChart.resize();
    });
}

// 初始化WebSocket连接
function initWebSocket() {
    ws = new WebSocket(WS_BASE_URL);
    
    ws.onopen = function() {
        console.log('WebSocket连接已建立');
    };
    
    ws.onmessage = function(event) {
        try {
            const data = JSON.parse(event.data);
            updateUI(data);
            updateChart(data);
        } catch (error) {
            console.error('解析WebSocket数据失败:', error);
        }
    };
    
    ws.onclose = function() {
        console.log('WebSocket连接已关闭');
        // 尝试重连
        setTimeout(initWebSocket, 3000);
    };
    
    ws.onerror = function(error) {
        console.error('WebSocket错误:', error);
    };
}

// 加载系统状态
function loadSystemStatus() {
    fetch(`${API_BASE_URL}/status`)
        .then(response => response.json())
        .then(data => {
            updateUI(data);
            updateChart(data);
        })
        .catch(error => {
            console.error('获取系统状态失败:', error);
        });
}

// 更新UI
function updateUI(data) {
    document.getElementById('current-temp').textContent = `${data.current_temp.toFixed(1)} °C`;
    document.getElementById('target-temp').textContent = `${data.target_temp.toFixed(1)} °C`;
    document.getElementById('heating-status').textContent = data.heating ? '开启' : '关闭';
    document.getElementById('pid-output').textContent = `${data.pid_output.toFixed(1)} %`;
    document.getElementById('system-status').textContent = '运行中';
    
    // 更新温度输入框
    document.getElementById('temp-input').value = data.target_temp;
}

// 更新图表
function updateChart(data) {
    // 添加新数据
    const now = new Date();
    const timeLabel = `${now.getMinutes()}:${now.getSeconds()}`;
    
    tempHistory.push({
        current: data.current_temp,
        target: data.target_temp
    });
    timeLabels.push(timeLabel);
    
    // 限制数据点数量
    if (tempHistory.length > maxDataPoints) {
        tempHistory.shift();
        timeLabels.shift();
    }
    
    // 更新图表数据
    tempChart.setOption({
        xAxis: {
            data: timeLabels
        },
        series: [
            {
                name: '当前温度',
                data: tempHistory.map(item => item.current)
            },
            {
                name: '目标温度',
                data: tempHistory.map(item => item.target)
            }
        ]
    });
}

// 设置目标温度
function setTargetTemperature(temp) {
    fetch(`${API_BASE_URL}/set_temp`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ target: temp })
    })
    .then(response => response.json())
    .then(data => {
        console.log('设置目标温度成功:', data);
        loadSystemStatus();
    })
    .catch(error => {
        console.error('设置目标温度失败:', error);
    });
}

// 设置PID参数
function setPIDParameters(kp, ki, kd) {
    fetch(`${API_BASE_URL}/set_pid`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ kp, ki, kd })
    })
    .then(response => response.json())
    .then(data => {
        console.log('设置PID参数成功:', data);
    })
    .catch(error => {
        console.error('设置PID参数失败:', error);
    });
}

// 绑定事件
function bindEvents() {
    // 温度控制按钮
    document.getElementById('temp-down').addEventListener('click', function() {
        const input = document.getElementById('temp-input');
        let value = parseFloat(input.value) || 0;
        if (value > 0) {
            input.value = value - 1;
        }
    });
    
    document.getElementById('temp-up').addEventListener('click', function() {
        const input = document.getElementById('temp-input');
        let value = parseFloat(input.value) || 0;
        if (value < 400) {
            input.value = value + 1;
        }
    });
    
    // 设置温度按钮
    document.getElementById('set-temp-btn').addEventListener('click', function() {
        const input = document.getElementById('temp-input');
        const temp = parseFloat(input.value);
        if (!isNaN(temp) && temp >= 0 && temp <= 400) {
            setTargetTemperature(temp);
        }
    });
    
    // 设置PID参数按钮
    document.getElementById('set-pid-btn').addEventListener('click', function() {
        const kp = parseFloat(document.getElementById('pid-kp').value);
        const ki = parseFloat(document.getElementById('pid-ki').value);
        const kd = parseFloat(document.getElementById('pid-kd').value);
        
        if (!isNaN(kp) && !isNaN(ki) && !isNaN(kd)) {
            setPIDParameters(kp, ki, kd);
        }
    });
    
    // 温度曲线相关事件
    if (document.getElementById('add-curve-point')) {
        document.getElementById('add-curve-point').addEventListener('click', function() {
            addCurvePoint();
        });
    }
    
    if (document.getElementById('upload-curve')) {
        document.getElementById('upload-curve').addEventListener('click', function() {
            uploadCurve();
        });
    }
    
    if (document.getElementById('start-curve')) {
        document.getElementById('start-curve').addEventListener('click', function() {
            startCurve();
        });
    }
    
    if (document.getElementById('stop-curve')) {
        document.getElementById('stop-curve').addEventListener('click', function() {
            stopCurve();
        });
    }
}

// 添加曲线点
function addCurvePoint() {
    const time = parseInt(document.getElementById('curve-time').value) || 0;
    const temp = parseFloat(document.getElementById('curve-temp').value) || 0;
    
    if (time >= 0 && temp >= 0) {
        curvePoints.push({ time, temp });
        updateCurvePointsList();
    }
}

// 更新曲线点列表
function updateCurvePointsList() {
    const list = document.getElementById('curve-points-list');
    if (list) {
        list.innerHTML = '';
        curvePoints.forEach((point, index) => {
            const li = document.createElement('li');
            li.textContent = `时间: ${point.time}s, 温度: ${point.temp}°C`;
            list.appendChild(li);
        });
    }
}

// 上传温度曲线
function uploadCurve() {
    const curveName = document.getElementById('curve-name').value || 'default';
    
    fetch(`${API_BASE_URL}/curve/upload`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            name: curveName,
            points: curvePoints.map(p => ({ time: p.time, temp: p.temp }))
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok') {
            alert('温度曲线上传成功');
        } else {
            alert('温度曲线上传失败: ' + data.message);
        }
    })
    .catch(error => {
        console.error('上传温度曲线失败:', error);
        alert('上传温度曲线失败');
    });
}

// 开始温度曲线
function startCurve() {
    const curveName = document.getElementById('curve-name').value || 'default';
    
    fetch(`${API_BASE_URL}/curve/start`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ name: curveName })
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok') {
            alert('温度曲线开始执行');
            isCurveRunning = true;
        } else {
            alert('温度曲线启动失败: ' + data.message);
        }
    })
    .catch(error => {
        console.error('启动温度曲线失败:', error);
        alert('启动温度曲线失败');
    });
}

// 停止温度曲线
function stopCurve() {
    fetch(`${API_BASE_URL}/curve/stop`, {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok') {
            alert('温度曲线已停止');
            isCurveRunning = false;
        }
    })
    .catch(error => {
        console.error('停止温度曲线失败:', error);
        alert('停止温度曲线失败');
    });
}

// 获取温度曲线状态
function getCurveStatus() {
    fetch(`${API_BASE_URL}/curve/status`)
    .then(response => response.json())
    .then(data => {
        isCurveRunning = data.is_running;
        if (data.is_running && data.points) {
            curvePoints = data.points.map(p => ({ time: p.time, temp: p.temp }));
            updateCurvePointsList();
        }
    })
    .catch(error => {
        console.error('获取温度曲线状态失败:', error);
    });
}

// 页面加载完成后初始化
window.addEventListener('DOMContentLoaded', init);

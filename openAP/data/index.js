var app = new Vue({
	el: '#app',
	created() {
		// 初始化data状态
		this.initData()
	},
	data: {
		forms: {
			f1: {
				t: '光剑模式',
				c: '#55ffff',
				v: 'f1',
			},
			f2: {
				t: '动感光波',
				c1: '#ff5500',
				c2: '#ffaaff',
				v: 'f2',
			},
			f3: {
				t: '发射子弹',
				c: '#55ffff',
				v: 'f3',
			},
			f4: {
				t: '绘光魔法',
				v: 'f4'
			}
		},
		Stick: {
			Status: 0,
			Map: {
				0: "文件传输中",
				1: "文件传输失败",
				2: "文件解析中",
				3: "文件解析失败",
				4: "展示中",
				5: "展示完毕",
				6: "展示失败"
			}
		},
		filesize: 0,
		checked: 'f1',
		showModal: false, // 控制弹窗的显示和隐藏
		msg: '',
		B: 120,
		INTER: [] //定时器id
	},
	methods: {
		initData: function() {
			// 手机端太卡了，就不渲染特效了
			var userAgent = navigator.userAgent;
			if (userAgent.match(/Windows|Macintosh|iPad/i)) {
				document.getElementById("lizi").setAttribute("src", "lizi.js");
			};
			fetch('/index.json').then(response => response.json())
				.then(data => {
					this.forms = data.forms;
					this.checked = data.checked;
					this.B = data.B;
				})
				.catch(error => {
					this.msg = error
				});
		},
		stickStatus: function() {
			// 获取光辉棒状态
			axios.get("/StickStatus")
				.then(data => {
					let s = parseInt(data.data.currentStatus);
					if (app.$data.Stick.Status != s) {
						app.$data.Stick.Status = s;
						console.log(app.$data.Stick.Map[s]);
					}
					switch (s) { //失败或完成后关闭
						case 1:
						case 3:
						case 5:
						case 6: {
							setTimeout(() => {
								app.closeAlert();
							}, 1000);
							break;
						}
					}
				})
		},
		upform: function(form_obj) {
			const dataform = new FormData()
			dataform.append("form", JSON.stringify(form_obj))
			axios.post("/colorchange", dataform).then(data => {
				console.log(data)
				app.$data.msg = "[成功]"+data.data.message
			}).catch(err=>{
				app.$data.msg = "upform错误，状态："+err.status
			})
		},
		colorchange: function(pathf) {
			// 是否已被选中
			var f = this.$data.forms[pathf]
			if (f.v == this.checked && (f.v != 'f4')) {
				this.upform(f)
			}
		},
		BControl: function() {
			const val = this.B;
			if (val > 0 || val <= 255) {
				const dataform = new FormData()
				dataform.append("v", val)
				axios.post("/set_Brightness", dataform)
			}
		},
		// 启动单片机绘制图案
		draw: function() {
			msg = '开始绘制'
			this.showAlert();
			axios.get("/draw")
		},
		// 弹窗部分
		showAlert: function() {
			this.showModal = true;
			this.INTER.push(setInterval(this.stickStatus, 800))
		},
		closeAlert: function() {
			this.INTER.forEach(i => {
				clearInterval(i)
			})
			this.INTER = [];
			this.showModal = false;
		},

		knock: function(pathf) {
			var f = this.$data.forms[pathf]
			if (f.v == this.checked) {
				var data = JSON.stringify(this.$data);
				const dataform = new FormData()
				dataform.append("c", f.c);
				axios.post("/knock", dataform).then((data) => {
					app.$data.msg = "发射成功了"
				})
			}
		},
		showImg: function() {
			window.open('verisign.bmp', '_blank')
		},
		saveConfig: function() {
			var data = JSON.stringify(this.$data);
			const dataform = new FormData()
			dataform.append("config", data)
			axios.post("/saveConfig", dataform)
				.then(data => {
					console.log(data.data.message)
					app.$data.msg = data.data.message
				}).catch(err => {
					app.$data.msg = 'err:' + err.status
				})
		},
		handleFile: function(event) {
			var file = event.target.files[0];
			if (!file || !file.type.startsWith('image/')) {
				alert("Please select an image file！");
				return;
			}
			this.showAlert();
			const reader = new FileReader();
			reader.onload = function(event) {
				const img = new Image();
				img.onload = function() {
					const canvas = document.getElementById('outputCanvas');
					var ctx = canvas.getContext('2d')
					const newWidth = 144;
					const scaleFactor = newWidth / img.width;
					const newHeight = img.height * scaleFactor;
					canvas.width = newWidth;
					canvas.height = newHeight;
					alert("图片高度为" + newHeight)
					ctx.drawImage(img, 0, 0, newWidth, newHeight);
					const dataform = new FormData();
					CanvasToBMP.toBlob(canvas, function(blob) {
						app.$data.filesize = (blob.size / 1024 / 8).toFixed(3);
						dataform.append('img', blob);
						axios({
							method: 'post',
							url: "/img",
							data: dataform,
							tumeout: 10000
						}).then(data => {
							app.closeAlert();
							this.msg = data.data.message
						}).catch(error => {
							app.$data.msg = "upfile_err:"+error.status
							console.error('err:', error);
							app.closeAlert()
							alert("传输错误")
						}).finally(() => {
							event.target.v = '';
						});
					})
				};
				img.src = event.target.result;
			};
			reader.readAsDataURL(file);
		}
	},
})
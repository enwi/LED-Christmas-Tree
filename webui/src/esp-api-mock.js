// eslint-disable-next-line no-unused-vars
import { createServer } from 'miragejs'

const startTime = Date.now()
let status_data = {
	uptime: 1200,
	lights: {
		brightness: 8,
		speed: 2,
		effect: 1,
		effects: [
			"off",
			"solid",
			"twoColorChange",
			"gradientHorizontal",
			"gradientVertical",
			"rainbowHorizontal",
			"rainbowVertical",
			"runningLight",
			"twinkleFox"
		]
	},
	mqtt: {
		"status": "connected", //Can be either disabled or connected or disconnected
	},
	network: {
		wifi_client: {
			"status": "disabled", //Can be either disabled or connected or disconnected
			"ip": "111.222.333.444",
			"netmask": "255.255.255.0",
			"dns": "1.3.4.5"
		},
		wifi_ap: {
			"status": "enabled", //Can be either disabled or enabled
			"ip": "111.222.333.444",
		},
		mac: "00:DE:AD:BE:EF:00",
	},
}

let config_data = {
	mqtt: {
		enabled: false,
		server: "192.168.1.74",
		port: 1883,
		id: "ESP8266-LedChristmasTree",
		user: "xxx",
		password: "xxx",
		// "topic_in": "/ESP00/cmd",
		// "topic_out": "/ESP00/msg",
	},
	wifi: {
		client_enabled: false,
		client_dhcp_enabled: true,
		client_ssid: "YourWifi-AP",
		client_password: "inputyourown",
		client_ip: "192.168.0.200",
		client_mask: "255.255.255.0",
		client_gateway: "192.168.0.1",
		client_dns: "192.168.0.1",
		ap_enabled: true,
		ap_ssid: "LedChristmasTree",
		ap_password: "",
		// ap_ip: "192.168.4.1",
		// ap_network: "192.168.4.0",
		// ap_mask: "255.255.255.0"
	},
}



export function makeServer() {
	createServer({
		routes() {
			this.post("/api/set_leds", (schema, request) => {
				let attrs = JSON.parse(request.requestBody)
				status_data.lights.brightness = attrs.brightness
				status_data.lights.speed = attrs.speed
				status_data.lights.effect = attrs.effect
				return "OK"
			})

			this.post("/api/config", (schema, request) => {
				config_data = JSON.parse(request.requestBody)
				return "OK"
			})

			this.get("/api/config", () => {
				return config_data
			})

			this.get("/api/status", () => {
				status_data.uptime = (Date.now() - startTime) / 1000
				return status_data
			})

			this.passthrough();
			this.passthrough('https://raw.githubusercontent.com/**');
		},
	})

}
import Vue from 'vue'
import App from './App.vue'
import VueRouter from 'vue-router'
import { router } from './router';

if (process.env.NODE_ENV === "development") {
	import('./esp-api-mock')
		.then((file) => {
			file.makeServer()
		});
}

Vue.use(VueRouter)
Vue.config.productionTip = false

import { capitalize, sec_to_human } from "./utils"

Vue.filter('capitalize', capitalize)
Vue.filter('sec_to_human', sec_to_human)

new Vue({
	router,
	render: h => h(App),
}).$mount('#app')

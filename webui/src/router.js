import Router from 'vue-router';

import Home from './components/PageHome'
import Config from './components/PageSettings'

export const router = new Router({
	mode: 'history',
	routes: [
		{ name: "Home", path: '/home', component: Home },
		{ name: "Config", path: '/config', component: Config },

		// otherwise redirect to home
		{ path: '*', redirect: '/home' }
	]
});
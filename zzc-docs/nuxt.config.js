const pkg = require('./package')

module.exports = {
  server: {
    host: '0.0.0.0'
  },
  mode: 'universal',
  head: {
    title: 'ZZC',
    meta: [
      { charset: 'utf-8' },
      { name: 'viewport', content: 'width=device-width, initial-scale=1' },
      { hid: 'description', name: 'description', content: pkg.description }
    ],
    link: [
      { rel: 'icon', type: 'image/x-icon', href: '/favicon.ico' }
    ]
  },
  loading: { color: '#ffd42a' },
  css: [
    { src: 'normalize.css' }
  ],
  plugins: [
    // { src: '~/plugins/plugin-name', ssr: false }
  ],
  modules: [
    '@nuxtjs/axios',
    '@nuxtjs/pwa',
    'nuxt-svg'
  ],
  axios: {
    // See https://github.com/nuxt-community/axios-module#options
  },
  build: {
    extend(config, ctx) {

    }
  }
}

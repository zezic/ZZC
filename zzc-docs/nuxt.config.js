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
    'nuxt-svg',
    ['nuxt-i18n', {
      strategy: 'prefix_and_default',
      defaultLocale: 'en',
      locales: [
        { code: 'en',
          iso: 'en-US' },
        { code: 'ru',
          iso: 'ru-RU' }
      ],
      vueI18n: {
        fallbackLocale: 'en',
        messages: {
          en: {
            menu: {
              all: 'All',
              free: 'Free',
              paid: 'Paid',
              author: 'Sergey Ukolov'
            },
            relatedModules: 'Related Modules',
            free: 'Free'
          },
          ru: {
            menu: {
              all: 'Все',
              free: 'Бесплатные',
              paid: 'Платные',
              author: 'Сергей Уколов'
            },
            relatedModules: 'Модули по теме',
            free: 'Бесплатно'
          }
        }
      }
    } ]
  ],
  axios: {
    browserBaseURL: '/'
  },
  router: {
    scrollBehavior: function (to, from, savedPosition) {
      // if the returned position is falsy or an empty object,
      // will retain current scroll position.
      let position = false

      // if no children detected
      if (to.matched.length < 2) {
        // scroll to the top of the page
        position = { x: 0, y: 0 }
      } else if (to.matched.some((r) => r.components.default.options.scrollToTop)) {
        // if one of the children has scrollToTop option set to true
        position = { x: 0, y: 0 }
      }

      // savedPosition is only available for popstate navigations (back button)
      if (savedPosition) {
        position = savedPosition
      }
      if (to.hash && document.querySelector(to.hash)) {
        // scroll to anchor by returning the selector
        position = { selector: to.hash }
      }
      return position
    }
  },
  build: {
    extend(config, ctx) {
    }
  }
}

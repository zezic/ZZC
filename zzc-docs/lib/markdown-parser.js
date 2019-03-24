'use strict';

import marked from 'marked'

function isZzcDoc (token) {
  return token.type === 'html' && token.text.startsWith('<!---')
}

function parseZzcDoc (text) {
  const lines = text.split('\n')
  return lines.reduce((acc, line) => {
    if (line.includes(':')) {
      const [ key, value ] = line.split(':').map(str => str.trim())
      acc[key] = value
    }
    return acc
  }, {})
}

class Blueprint {
  constructor (doc) {
    this.blueprint = doc.blueprint
    this.preview = doc.preview
  }
}

class Affixed {
  constructor (doc) {
    this.type = doc.affixed
    if (this.type === 'blueprint') {
      this.affixed = new Blueprint(doc)
    }
  }
}

class TokenScraper {
  scrapeTokens (tokens, items, haltFunc) {
    items = items || []
    if (tokens.length === 0) { return { tokens, items } }
    const token = tokens.shift()
    if (haltFunc && haltFunc(token)) {
      console.log('HALT:', token)
      return {
        tokens,
        items
      }
    }
    if (isZzcDoc(token)) {
      const doc = parseZzcDoc(token.text)
      if (doc.start) {
        const newItem = sectionFactory(doc)
        tokens = newItem.collectTokens(tokens)
        items.push(newItem)
        return this.scrapeTokens(tokens, items, haltFunc)
      }
    } else if (token.type.endsWith('_start')) {
      const sectionType = token.type.split('_start')[0]
      const newItem = new MarkdownSection(sectionType)
      tokens = newItem.collectTokens(tokens)
      items.push(newItem)
      return this.scrapeTokens(tokens, items, haltFunc)
    }
    items.push(token)
    return this.scrapeTokens(tokens, items, haltFunc)
  }
}

class Section extends TokenScraper {
  constructor (doc) {
    super()
    this.type = doc.start
    this.items = []
  }
  collectTokens (tokens) {
    const result = this.scrapeTokens(tokens, [], this.checkForTerminator)
    this.items = result.items
    return result.tokens
  }
  checkForTerminator = (token) => {
    if (!isZzcDoc(token)) { return false }
    const doc = parseZzcDoc(token.text)
    return doc.end && doc.end === this.type
  }
}

class MarkdownSection extends Section {
  constructor (type) {
    super({ start: type })
  }
  checkForTerminator = (token) => {
    const endMarker = `${this.type}_end`
    return token.type && token.type === endMarker
  }
}

class Affixing extends Section {
  constructor (doc) {
    super(doc)
    this.affixed = new Affixed(doc)
  }
}

class Legend extends Section {
  constructor (doc) {
    super(doc)
  }
}

function sectionFactory (doc) {
  if (doc.start === 'affixing') { return new Affixing(doc) }
  if (doc.start === 'legend') { return new Legend(doc) }
}

class MarkdownParser extends TokenScraper {
  parse (markdown) {
    const tokens = marked.lexer(markdown)
    const result = this.scrapeTokens(tokens)
    return result.items
  }
}

export default MarkdownParser

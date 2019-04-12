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
      return {
        tokens,
        items
      }
    }
    const isZzcSection = isZzcDoc(token) && parseZzcDoc(token.text).start
    const isSectionStart = isZzcSection || token.type.endsWith('_start')
    if (isSectionStart) {
      const newItem = (this.sectionFactory && this.sectionFactory(token)) || sectionFactory(token)
      tokens = newItem.collectTokens(tokens)
      items.push(newItem)
      return this.scrapeTokens(tokens, items, haltFunc)
    }
    items = this.consumeItem(items, token)
    return this.scrapeTokens(tokens, items, haltFunc)
  }
  consumeItem (items, token) {
    items.push(token)
    return items
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

class LegendListItem extends MarkdownSection {
  constructor (type) {
    super(type)
  }
  consumeItem (items, token) {
    if (isZzcDoc(token)) {
      const doc = parseZzcDoc(token.text)
      this.options = doc
    } else {
      items.push(token)
    }
    return items
  }
}

class LegendList extends MarkdownSection {
  constructor (type) {
    super(type)
  }
  sectionFactory (token) {
    const sectionType = token.type.split('_start')[0]
    if (sectionType === 'list_item') { return new LegendListItem(sectionType) }
  }
}

class LegendGroup extends Section {
  constructor (doc) {
    super(doc)
    this.slug = doc.slug
  }
  sectionFactory (token) {
    const sectionType = token.type.split('_start')[0]
    if (sectionType === 'list') { return new LegendList(sectionType) }
  }
}

class Legend extends Section {
  constructor (doc) {
    super(doc)
  }
  sectionFactory (token) {
    if (isZzcDoc(token)) {
      const doc = parseZzcDoc(token.text)
      if (doc.start === 'legend-group') { return new LegendGroup(doc) }
    }
  }
  consumeItem (items, token) {
    if (token.type === 'heading') {
      this.currentGroupTitle = token.text
    } else {
      items.push(token)
    }
    return items
  }
}

function sectionFactory (token) {
  if (isZzcDoc(token)) {
    const doc = parseZzcDoc(token.text)
    if (doc.start === 'affixing') { return new Affixing(doc) }
    if (doc.start === 'legend') { return new Legend(doc) }
  } else {
    const sectionType = token.type.split('_start')[0]
    return new MarkdownSection(sectionType)
  }
}

class MarkdownParser extends TokenScraper {
  parse (markdown) {
    const tokens = marked.lexer(markdown)
    const result = this.scrapeTokens(tokens)
    return result.items
  }
}

export default MarkdownParser

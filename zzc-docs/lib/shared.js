export function makeGroupItemSlug (item, group) {
  return `${group.title.toLowerCase()}-${item.options.slug}`
}


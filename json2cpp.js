function jsontocpp(obj, tab = "  ", indents = 0) {
  if (obj === null) {
    return "nullptr";
  } else if (Array.isArray(obj)) {
    if (obj.length == 0) return "{}";
    let str = "{\n";
    for (const v of obj) {
      str += `${tab.repeat(indents + 1)}${jsontocpp(v, tab, indents + 1)},\n`;
    }
    str += `${tab.repeat(indents)}}`;
    return str;
  } else if (typeof obj == "object") {
    let str = "{\n";
    for (const key in obj) {
      str += `${tab.repeat(indents + 1)}{${JSON.stringify(key)}, ${jsontocpp(obj[key], tab, indents + 1)}},\n`;
    }
    str += `${tab.repeat(indents)}}`;
    return str;
  } else {
    return JSON.stringify(obj);
  }
}

fn main() {
    println!("cargo:rustc-link-lib=static=parser");
    cc::Build::new()
        .file("../src/parser.c")
        .file("../src/html.c")
        .file("../src/punct.c")
        .file("../src/affixes.c")
        .file("../src/typifier.c")
        .file("../src/util.c")
        .compile("parser");
}

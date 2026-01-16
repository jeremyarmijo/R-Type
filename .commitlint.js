module.exports = {
  extends: ["@commitlint/config-conventional"],
  rules: {
    "type-case": [2, "always", "lower-case"],
    "type-enum": [
      2,
      "always",
      [
        "build",
        "ci",
        "docs",
        "feat",
        "fix",
        "perf",
        "refactor",
        "revert",
        "style",
        "test",
      ],
    ],
    "scope-empty": [2, "never"],
    "scope-enum": [
      2,
      "always",
      [
        "documentation",
        "jobsystem",
        "engine",
        "network",
        "gamedesign",
        "cross-platform",
      ],
    ],
  },
};

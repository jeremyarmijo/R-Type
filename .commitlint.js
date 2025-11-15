module.exports = {
  extends: ['@commitlint/config-conventional'],
  rules: {
    'type-enum': [
      2,
      'always',
      [
        'feat',     // Nouvelle fonctionnalité
        'fix',      // Correction de bug
        'docs',     // Documentation
        'style',    // Changements de style (formatage, etc.)
        'refactor', // Refactorisation de code
        'test',     // Ajout/modification de tests
        'chore',    // Tâches de maintenance
      ],
    ],
  },
};

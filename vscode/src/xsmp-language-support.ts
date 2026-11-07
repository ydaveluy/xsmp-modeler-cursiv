const smpMirrorScheme = 'xsmp-smp';

export const xsmpLanguageIds = [
    'xsmpproject',
    'xsmpcat',
    'xsmpcfg',
    'xsmpasb',
    'xsmplnk',
    'xsmpsed',
] as const;

const xsmpReadonlySchemes = [
    'xsmp',
    smpMirrorScheme,
] as const;

export function createXsmpDocumentSelector() {
    return [
        ...xsmpLanguageIds.map(language => ({ scheme: 'file', language })),
        ...xsmpReadonlySchemes.flatMap(scheme => xsmpLanguageIds.map(language => ({ scheme, language }))),
    ];
}

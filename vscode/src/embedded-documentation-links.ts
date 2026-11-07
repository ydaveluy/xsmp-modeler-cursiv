export interface XsmpEmbeddedDocumentationTarget {
    readonly page: string;
    readonly anchor: string;
}

interface XsmpEmbeddedDocumentationLanguageEntry {
    readonly page: string;
    readonly anchorsByKeyword: Readonly<Record<string, string | undefined>>;
}

const embeddedDocumentationByLanguage: Readonly<Partial<Record<string, XsmpEmbeddedDocumentationLanguageEntry>>> = {
    xsmpproject: {
        page: 'languages/xsmpproject.md',
        anchorsByKeyword: {
            project: 'root-syntax',
            using: 'root-syntax',
            source: 'source',
            dependency: 'dependency',
            profile: 'profile',
            tool: 'tool',
        },
    },
    xsmpcat: {
        page: 'languages/xsmpcat.md',
        anchorsByKeyword: {
            catalogue: 'file-structure',
            namespace: 'namespaces',
            public: 'type-declarations',
            protected: 'type-declarations',
            private: 'type-declarations',
            abstract: 'type-declarations',
            input: 'field',
            output: 'field',
            transient: 'field',
            readonly: 'property',
            writeonly: 'property',
            readwrite: 'property',
            struct: 'struct',
            class: 'class',
            exception: 'exception',
            interface: 'interface',
            model: 'model',
            service: 'service',
            array: 'array',
            using: 'using',
            integer: 'integer',
            float: 'float',
            event: 'event',
            string: 'string',
            primitive: 'primitive',
            native: 'native',
            attribute: 'attribute',
            enum: 'enum',
            constant: 'constant',
            field: 'field',
            property: 'property',
            def: 'def',
            association: 'association',
            container: 'container',
            reference: 'reference',
            entrypoint: 'entrypoint',
            eventsink: 'eventsink',
            eventsource: 'eventsource',
            throws: 'def',
            get: 'property',
            set: 'property',
            in: 'def',
            out: 'def',
            inout: 'def',
            extends: 'type-declarations',
            implements: 'type-declarations',
        },
    },
    xsmpcfg: {
        page: 'languages/xsmpcfg.md',
        anchorsByKeyword: {
            configuration: 'root-structure',
            include: 'include',
            at: 'include',
            unsafe: 'paths',
        },
    },
    xsmpasb: {
        page: 'languages/xsmpasb.md',
        anchorsByKeyword: {
            assembly: 'root-structure',
            configure: 'configure-blocks',
            subscribe: 'global-event-subscriptions',
            property: 'property',
            call: 'call',
            event: 'event-link',
            field: 'field-link',
            interface: 'interface-link',
            link: 'local-links',
            using: 'sub-assembly-instances',
            config: 'sub-assembly-instances',
            unsafe: 'paths',
        },
    },
    xsmplnk: {
        page: 'languages/xsmplnk.md',
        anchorsByKeyword: {
            link: 'root-structure',
            for: 'root-structure',
            event: 'event-link',
            field: 'field-link',
            interface: 'interface-link',
            unsafe: 'paths',
        },
    },
    xsmpsed: {
        page: 'languages/xsmpsed.md',
        anchorsByKeyword: {
            schedule: 'root-structure',
            epoch: 'epoch-event',
            mission: 'mission-event',
            task: 'tasks',
            on: 'tasks',
            call: 'call',
            property: 'property',
            transfer: 'transfer',
            trig: 'trig',
            execute: 'execute',
            async: 'emit',
            emit: 'emit',
            event: 'events',
            simulation: 'simulation-event',
            zulu: 'zulu-event',
            until: 'global-event-triggered-event',
            using: 'global-event-triggered-event',
            delay: 'global-event-triggered-event',
            cycle: 'global-event-triggered-event',
            repeat: 'global-event-triggered-event',
            unsafe: 'paths',
        },
    },
};

export function getEmbeddedDocumentationTarget(
    languageId: string,
    keyword: string,
    lineText?: string,
): XsmpEmbeddedDocumentationTarget | undefined {
    const entry = embeddedDocumentationByLanguage[languageId];
    if (!entry) {
        return undefined;
    }

    const normalizedKeyword = keyword.trim().toLowerCase();
    const anchor = lineAnchors[languageId]?.(normalizedKeyword, (lineText ?? '').toLowerCase()) ?? entry.anchorsByKeyword[normalizedKeyword];
    return anchor === undefined ? undefined : { page: entry.page, anchor };
}

const scheduleEventForms: readonly (readonly [RegExp, string])[] = [
    [/^\s*event\b.*\bmission\b/, 'mission-event'],
    [/^\s*event\b.*\bepoch\b/, 'epoch-event'],
    [/^\s*event\b.*\bsimulation\b/, 'simulation-event'],
    [/^\s*event\b.*\bzulu\b/, 'zulu-event'],
    [/^\s*event\b.*\bon\b/, 'global-event-triggered-event'],
];

// The anchors that depend on the rest of the line. Undefined leaves the keyword
// to the table above.
const lineAnchors: Readonly<Partial<Record<string, (keyword: string, line: string) => string | undefined>>> = {
    xsmpasb: (keyword, line) => {
        const link = ['event', 'field', 'interface', 'link'].includes(keyword) ? /\b(event|field|interface)\s+link\b/.exec(line) : null;
        return link ? `${link[1]}-link` : undefined;
    },
    xsmplnk: (keyword, line) => {
        const link = keyword === 'link' ? /^\s*(event|field|interface)\s+link\b/.exec(line) : null;
        return link ? `${link[1]}-link` : undefined;
    },
    xsmpsed: (keyword, line) => {
        const forms = keyword === 'on' ? scheduleEventForms.slice(-1) : ['event', 'cycle', 'repeat'].includes(keyword) ? scheduleEventForms : [];
        return forms.find(([form]) => form.test(line))?.[1];
    },
};

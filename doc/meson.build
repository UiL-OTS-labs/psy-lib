md_files = files(
    'introduction.md'
)

if get_option('documentation')

    gidocgen = find_program('gi-docgen', required : true)

    toml_conf = configuration_data()
    toml_conf.set('version', meson.project_version())

    doc_project_description = configure_file(
        input: 'psy-doc.toml.in',
        output: 'psy-doc.toml',
        configuration : toml_conf
    )

    gidocgen_common_args = [
        '--quiet',
        '--no-namespace-dir',
    ]

    documentation_target = custom_target (
        'documentation',
        input: [doc_project_description, gir_target],
        output : 'doc',
        command: [
            gidocgen,
            'generate',
            gidocgen_common_args,
            '--add-include-path=@0@'.format(meson.current_build_dir() / '../../psy'),
            '--output-dir=@0@'.format(meson.current_build_dir()),
            '--config=@INPUT0@',
            '--content-dir=@0@'.format(meson.current_build_dir()),
            '--content-dir=@0@'.format(meson.current_source_dir()),
            '@INPUT1@'
        ],
        depends : [],
        depend_files : [ md_files ],
        build_by_default : true
    )

endif

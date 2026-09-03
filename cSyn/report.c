#include "report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Escaping helpers ----

static void html_escape(FILE *out, const char *text, int len) {
    if (!text) return;
    for (int i = 0; i < len; i++) {
        switch (text[i]) {
            case '&': fputs("&amp;", out); break;
            case '<': fputs("&lt;", out); break;
            case '>': fputs("&gt;", out); break;
            case '"': fputs("&quot;", out); break;
            case '\'': fputs("&#39;", out); break;
            default: fputc(text[i], out); break;
        }
    }
}

static void json_escape(FILE *out, const char *text, int len) {
    if (!text) return;
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        switch (c) {
            case '"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if (c < 0x20) fprintf(out, "\\u%04x", c);
                else fputc(c, out);
                break;
        }
    }
}

static const char *severity_name(Severity sev) {
    switch (sev) {
        case SEV_ERROR: return "Error";
        case SEV_WARNING: return "Warning";
        case SEV_INFO: return "Info";
        case SEV_STYLE: return "Style";
    }
    return "Unknown";
}

static void count_diags(DiagnosticList *diags, int *errors, int *warnings,
                        int *infos, int *styles) {
    *errors = *warnings = *infos = *styles = 0;
    for (int i = 0; i < diags->count; i++) {
        switch (diags->items[i].severity) {
            case SEV_ERROR: (*errors)++; break;
            case SEV_WARNING: (*warnings)++; break;
            case SEV_INFO: (*infos)++; break;
            case SEV_STYLE: (*styles)++; break;
        }
    }
}

// ---- HTML report ----

void generate_html_report(const char *filename, const char *source,
                          DiagnosticList *diags, FILE *out) {
    int errors, warnings, infos, styles;
    count_diags(diags, &errors, &warnings, &infos, &styles);
    int total = diags->count;

    fputs("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
          "<meta charset=\"utf-8\">\n"
          "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
          "<title>Code Analysis Report</title>\n"
          "<style>\n"
          "body{font-family:-apple-system,Segoe UI,Roboto,Arial,sans-serif;"
          "margin:0;background:#f5f6f8;color:#1f2328;}\n"
          "header{background:#24292e;color:#fff;padding:20px 28px;}\n"
          "header h1{margin:0;font-size:20px;}\n"
          "header p{margin:6px 0 0;opacity:.8;font-size:13px;}\n"
          "main{max-width:1000px;margin:24px auto;padding:0 16px;}\n"
          ".stats{display:flex;gap:12px;margin:0 0 24px;flex-wrap:wrap;}\n"
          ".stat{flex:1;min-width:120px;background:#fff;border:1px solid #d8dee4;"
          "border-radius:8px;padding:14px;text-align:center;}\n"
          ".stat b{display:block;font-size:26px;}\n"
          ".stat span{font-size:12px;color:#57606a;text-transform:uppercase;"
          "letter-spacing:.05em;}\n"
          ".err b{color:#cf222e;}.warn b{color:#bf8700;}.info b{color:#0969da;}"
          ".style b{color:#8250df;}.tot b{color:#1f2328;}\n"
          "h2{font-size:16px;border-bottom:1px solid #d8dee4;padding-bottom:8px;}\n"
          ".diag{background:#fff;border:1px solid #d8dee4;border-radius:8px;"
          "margin:8px 0;padding:12px 16px;}\n"
          ".diag .sev{font-size:11px;font-weight:700;text-transform:uppercase;"
          "border-radius:4px;padding:2px 8px;margin-right:8px;}\n"
          ".sev-Error{background:#ffebe9;color:#cf222e;}\n"
          ".sev-Warning{background:#fff8c5;color:#9a6700;}\n"
          ".sev-Info{background:#ddf4ff;color:#0969da;}\n"
          ".sev-Style{background:#fbefff;color:#8250df;}\n"
          ".diag .loc{color:#57606a;font-size:12px;}\n"
          ".diag .msg{margin-top:4px;}\n"
          "pre.code{background:#0d1117;color:#c9d1d9;border-radius:8px;"
          "padding:16px;overflow:auto;font-size:13px;line-height:1.5;}\n"
          "pre.code .ln{display:inline-block;width:3em;color:#484f58;"
          "text-align:right;margin-right:12px;user-select:none;}\n"
          "footer{text-align:center;color:#57606a;font-size:12px;"
          "padding:24px 0 32px;}\n"
          "</style>\n"
          "</head>\n<body>\n"
          "<header>\n"
          "<h1>Code Analysis Report</h1>\n"
          "<p>File: ", out);
    html_escape(out, filename, (int)strlen(filename));
    fputs("</p>\n</header>\n<main>\n", out);

    // Stats
    fputs("<div class=\"stats\">\n"
          "<div class=\"stat err\"><b>", out);
    fprintf(out, "%d</b><span>Errors</span></div>\n", errors);
    fputs("<div class=\"stat warn\"><b>", out);
    fprintf(out, "%d</b><span>Warnings</span></div>\n", warnings);
    fputs("<div class=\"stat info\"><b>", out);
    fprintf(out, "%d</b><span>Info</span></div>\n", infos);
    fputs("<div class=\"stat style\"><b>", out);
    fprintf(out, "%d</b><span>Style</span></div>\n", styles);
    fputs("<div class=\"stat tot\"><b>", out);
    fprintf(out, "%d</b><span>Total</span></div>\n</div>\n", total);

    // Source (line-numbered, escaped)
    fputs("<h2>Source</h2>\n<pre class=\"code\">", out);
    {
        int lineno = 1;
        fprintf(out, "<span class=\"ln\">%d</span>", lineno);
        for (const char *p = source; p && *p; p++) {
            if (*p == '\n') {
                fputc('\n', out);
                lineno++;
                fprintf(out, "<span class=\"ln\">%d</span>", lineno);
            } else {
                html_escape(out, p, 1);
            }
        }
    }
    fputs("</pre>\n", out);

    // Diagnostics
    fputs("<h2>Diagnostics</h2>\n", out);
    if (diags->count == 0) {
        fputs("<p><em>No diagnostics found.</em></p>\n", out);
    }
    for (int i = 0; i < diags->count; i++) {
        Diagnostic *d = &diags->items[i];
        fputs("<div class=\"diag\">\n<span class=\"sev sev-", out);
        fputs(severity_name(d->severity), out);
        fputs("\">", out);
        fputs(severity_name(d->severity), out);
        fputs("</span>", out);
        if (d->line > 0) {
            fputs("<span class=\"loc\">Line ", out);
            fprintf(out, "%d, column %d</span>", d->line, d->column);
        }
        fputs("<div class=\"msg\">", out);
        html_escape(out, d->message, (int)strlen(d->message));
        fputs("</div>\n</div>\n", out);
    }

    fputs("</main>\n<footer>Generated by C-SyntaxChecker</footer>\n"
          "</body>\n</html>\n", out);
}

// ---- JSON report ----

void generate_json_report(const char *filename, const char *source,
                          DiagnosticList *diags, FILE *out) {
    int errors, warnings, infos, styles;
    count_diags(diags, &errors, &warnings, &infos, &styles);

    fputs("{\n  \"file\": \"", out);
    json_escape(out, filename, (int)strlen(filename));
    fputs("\",\n  \"source\": \"", out);
    json_escape(out, source ? source : "", (int)strlen(source ? source : ""));
    fputs("\",\n  \"summary\": {\n"
          "    \"errors\": ", out);
    fprintf(out, "%d,\n    \"warnings\": %d,\n    \"info\": %d,\n"
            "    \"style\": %d,\n    \"total\": %d\n  },\n",
            errors, warnings, infos, styles, diags->count);

    fputs("  \"diagnostics\": [\n", out);
    for (int i = 0; i < diags->count; i++) {
        Diagnostic *d = &diags->items[i];
        fputs("    {\n      \"severity\": \"", out);
        fputs(severity_name(d->severity), out);
        fputs("\",\n      \"line\": ", out);
        fprintf(out, "%d,\n      \"column\": %d,\n      \"message\": \"",
                d->line, d->column);
        json_escape(out, d->message, (int)strlen(d->message));
        fputs("\"\n    }", out);
        fputs(i < diags->count - 1 ? ",\n" : "\n", out);
    }
    fputs("  ]\n}\n", out);
}
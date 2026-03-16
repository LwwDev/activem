using System.Diagnostics;

var builder = WebApplication.CreateBuilder(args);
var app = builder.Build();

var samplerPath = app.Configuration["SamplerPath"] ?? "sampler.exe";

app.MapGet("/metrics/stream", async (HttpContext ctx, CancellationToken ct) =>
{
    ctx.Response.ContentType = "text/event-stream";
    ctx.Response.Headers.CacheControl = "no-cache";

    var proc = new Process
    {
        StartInfo = new ProcessStartInfo
        {
            FileName = samplerPath,
            RedirectStandardOutput = true,
            UseShellExecute = false,
        }
    };

    proc.Start();

    while (!ct.IsCancellationRequested)
    {
        var line = await proc.StandardOutput.ReadLineAsync(ct);
        if (line == null) break;

        await ctx.Response.WriteAsync($"data: {line}\n\n", ct);
        await ctx.Response.Body.FlushAsync(ct);
    }

    proc.Kill();
});

app.Run();

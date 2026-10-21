use std::env;
use std::ffi::OsStr;
use std::path::PathBuf;
use std::process::{Command, ExitStatus};

fn run_command<I, S>(program: &str, args: I) -> Result<(), String>
where
    I: IntoIterator<Item = S>,
    S: AsRef<OsStr>,
{
    let args_vec: Vec<_> = args.into_iter().map(|s| s.as_ref().to_owned()).collect();
    let status = Command::new(program)
        .args(&args_vec)
        .status()
        .map_err(|e| format!("failed to run {}: {}", program, e))?;

    if !status.success() {
        return Err(format_status(program, &args_vec, status));
    }

    Ok(())
}

fn format_status(program: &str, args: &[std::ffi::OsString], status: ExitStatus) -> String {
    let args_str = args
        .iter()
        .map(|a| a.to_string_lossy())
        .collect::<Vec<_>>()
        .join(" ");
    format!("command failed: {} {} (exit: {})", program, args_str, status)
}

fn project_root() -> Result<PathBuf, String> {
    let manifest_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    manifest_dir
        .parent()
        .and_then(|p| p.parent())
        .map(PathBuf::from)
        .ok_or_else(|| "unable to determine project root".to_string())
}

fn run_make_target(target: &str) -> Result<(), String> {
    let root = project_root()?;
    let status = Command::new("make")
        .arg(target)
        .current_dir(root)
        .status()
        .map_err(|e| format!("failed to invoke make {}: {}", target, e))?;

    if !status.success() {
        return Err(format!("make {} failed with status {}", target, status));
    }

    Ok(())
}

fn usage() {
    eprintln!(
        "usage: cargo xtask <command>\n\ncommands:\n  check\n  build\n  build-bios\n  build-uefi\n  build-iso\n  run-bios\n  run-uefi\n  clean"
    );
}

fn main() {
    let mut args = env::args().skip(1);
    let Some(cmd) = args.next() else {
        usage();
        std::process::exit(2);
    };

    if args.next().is_some() {
        usage();
        std::process::exit(2);
    }

    let result = match cmd.as_str() {
        "check" => run_make_target("check"),
        "build" => run_make_target("build"),
        "build-bios" => run_make_target("build-bios"),
        "build-uefi" => run_make_target("build-uefi"),
        "build-iso" => run_make_target("build-iso"),
        "run-bios" => run_make_target("run-bios"),
        "run-uefi" => run_make_target("run-uefi"),
        "clean" => run_make_target("clean"),
        "_self-test-run-command" => run_command("true", ["" ]),
        _ => {
            usage();
            Err(format!("unknown command: {}", cmd))
        }
    };

    if let Err(err) = result {
        eprintln!("xtask error: {}", err);
        std::process::exit(1);
    }
}

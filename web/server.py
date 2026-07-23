import subprocess
from pathlib import Path

from flask import Flask, jsonify, render_template, request

EXE_PATH = Path(__file__).resolve().parent.parent / "refactorer.exe"

app = Flask(__name__)


def run_refactorer(path, apply_option=None):
    args = [str(EXE_PATH), path]
    if apply_option is not None:
        args += ["--apply", str(apply_option)]

    try:
        result = subprocess.run(args, capture_output=True, text=True, timeout=60)
    except subprocess.TimeoutExpired:
        return None, "Analiza a durat prea mult (timeout)."
    except OSError as exc:
        return None, f"Nu am putut porni refactorer.exe: {exc}"

    if result.returncode != 0:
        return None, result.stderr.strip() or result.stdout.strip() or "Eroare necunoscută."

    return result.stdout, None


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/analyze", methods=["POST"])
def analyze():
    data = request.get_json(force=True, silent=True) or {}
    path = str(data.get("path", "")).strip()

    if not path:
        return jsonify(success=False, error="Introdu un path către folder."), 400
    if not Path(path).is_dir():
        return jsonify(success=False, error="Folderul nu există."), 400

    output, error = run_refactorer(path)
    if error:
        return jsonify(success=False, error=error), 400

    return jsonify(success=True, output=output)


@app.route("/api/apply", methods=["POST"])
def apply():
    data = request.get_json(force=True, silent=True) or {}
    path = str(data.get("path", "")).strip()
    option = str(data.get("option", "")).strip()

    if not path:
        return jsonify(success=False, error="Introdu un path către folder."), 400
    if not Path(path).is_dir():
        return jsonify(success=False, error="Folderul nu există."), 400
    if option not in ("1", "2", "3", "4"):
        return jsonify(success=False, error="Opțiune invalidă."), 400

    output, error = run_refactorer(path, apply_option=option)
    if error:
        return jsonify(success=False, error=error), 400

    return jsonify(success=True, output=output)


if __name__ == "__main__":
    if not EXE_PATH.exists():
        raise SystemExit(f"Nu găsesc {EXE_PATH}. Compilează refactorer.exe întâi.")
    app.run(host="127.0.0.1", port=5000, debug=True)

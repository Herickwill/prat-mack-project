resource "aws_s3_object" "frontend_index" {
  bucket       = aws_s3_bucket.frontend_bucket.bucket
  key          = "index.html"
  source       = "${path.module}/../frontend/public/index.html"
  content_type = "text/html"
  etag         = filemd5("${path.module}/../frontend/public/index.html")
  tags = {
    Project = "agua-monitoramento"
  }
}

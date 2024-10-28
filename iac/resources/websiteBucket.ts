import { TerraformStack, TerraformVariable } from "cdktf";
import { S3Bucket } from "@cdktf/provider-aws/lib/s3-bucket";
import { S3Object } from "@cdktf/provider-aws/lib/s3-object";
import * as path from "path";
import * as glob from "glob";
import * as mime from "mime-types";

function configureBucket(
  stack: TerraformStack,
  websiteBucketName: TerraformVariable,
  resourceTags?: any,
): S3Bucket {
  // Make an S3 bucket for the static website
  const staticWebsiteStorageBucket = new S3Bucket(stack, "websiteBucket", {
    bucket: websiteBucketName.toString(),
    forceDestroy: true,
    tags: {
      ...resourceTags,
    },
  });
  return staticWebsiteStorageBucket;
}

function configureBucketData(
  stack: TerraformStack,
  staticWebsiteStorageBucket: S3Bucket,
) {
  // Upload the static website files to the bucket
  const files = glob.sync("../dist/**/*", { absolute: false, nodir: true });
  for (const file of files) {
    const key = file.replace(`../dist/`, "");
    new S3Object(stack, `aws_s3_bucket_object_${key}`, {
      dependsOn: [staticWebsiteStorageBucket], // Wait untill the bucket is created
      key: key, // Using relative path for folder structure on S3
      bucket: staticWebsiteStorageBucket.bucket,
      source: path.resolve(file), // Using absolute path to upload
      etag: `${Date.now()}`,
      contentType: mime.contentType(path.extname(file)) || undefined, // Set the content-type for each object
    });
  }
}

export function configureWebsite(
  stack: TerraformStack,
  websiteBucketName: TerraformVariable,
  resourceTags?: any,
): S3Bucket {
  const staticWebsiteStorageBucket = configureBucket(
    stack,
    websiteBucketName,
    resourceTags,
  );
  configureBucketData(stack, staticWebsiteStorageBucket);
  return staticWebsiteStorageBucket;
}
